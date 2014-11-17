extern "C"
{
#include "unpthread.h"
}
#include "server.h"
#include "master.h"

static sigjmp_buf jmpbuf;
struct ARGS //struct of arguments, used for passing arguments to thread
{
    Server *srv;
    char *buf;
    Request *req;
    int connfd;
    
    ARGS(Server *s, char *buffer, Request *request, int conn_fd)
    {
        srv = s;
        buf = buffer;
        req = request;
        connfd = conn_fd;
    }
};

struct ARGS_PING
{
    Server *s;
    Master *ms;
    
    ARGS_PING(Server *srv, Master *master)
    {
        s = srv;
        ms = master;
    }
};

static void *Sync_Hist(void *);
static void *Sync_Sent(void *);
static void *Sync(void *);
static void ParseIpaddr(string input_str, struct sockaddr_in &cliaddr);
static void *Reply(void *);
static void *Ping(void *);
static void *Proc_master_notification(void *);
static void Sync_procTrans(Server *);
static void Sync_sentTrans(Server *);
static void sig_int_handle(int signo) //handle the signal of SIGINT
{
    siglongjmp(jmpbuf, 1);
}
static void sig_term_handle(int signo) //handle the signal of SIGTERM
{
    siglongjmp(jmpbuf, 2);
}

int main(int argc, char **argv)
{
    struct sockaddr_in srvaddr, cliaddr;
    int sockfd_tcp, sockfd_udp, connfd, maxfd;
	ifstream fin;
	fin.open(argv[2]);
	string input_str;
	char *input;
	Master *ms = new Master();
    Server *s = new Server();
	int index = atoi(argv[1]);
    int i = 0;
	int flag_server = 0, flag_next = 0, flag_master = 0;
	const int on = 1;
    fd_set allset, rset;
    socklen_t len = sizeof(struct sockaddr_in);
    char buf[MAXLINE];
    int msg_count = 0, loss_frequency = 0;
	while(fin.good())
	{
		getline(fin, input_str);
		if (input_str == "\0")
			continue;
     	input = const_cast<char*>(input_str.c_str());
		if (!strncmp(input, "//Master", 8))
		{
			flag_master = 1;
			continue;
		}
        else if (!strncmp(input, "//Server", 8))
        {
            flag_server = 1;
            continue;
        }
        else if (!strncmp(input, "//Client", 8))
            break;
        if (flag_master)
		{
			ms->Init(input);
			flag_master = 0;
			continue;
		}
		else if (flag_server && ((++i) == index))
		{
			s->InitServ(input_str);
            flag_server = 0;
            flag_next = 1;
            continue;
		}
        else if (flag_next)
        {
            stringstream str;
            int next_bankname;
            str<<input_str[0];
            str>>next_bankname;
            if (next_bankname == s->GetbankName())
            {
                Server *next_srv = new Server();
                next_srv->InitServ(input_str);
                if (!next_srv->Getdelay())
                    s->Setnext(next_srv);
            }
            break;
        }
	}
    if (s->Getdelay() > 100)
        exit(1);
    sleep(s->Getdelay());
	cout<<"----------------Server starting---------------"<<endl;
	cout<<s;
    s->Setsocket();
    sockfd_tcp = s->Getsockfd_tcp();
    sockfd_udp = s->Getsockfd_udp();
    listen(sockfd_tcp, LISTENQ);
    maxfd = max(sockfd_tcp,sockfd_udp) + 1;
    FD_ZERO(&allset);
    FD_SET(sockfd_tcp, &allset);
    FD_SET(sockfd_udp, &allset);
    pthread_t tid_ping;
    ARGS_PING args_ping(s, ms);
    Pthread_create(&tid_ping,NULL,&Ping,(void *)&args_ping);
    while(1)
    {
        Signal(SIGINT, sig_int_handle);
        Signal(SIGTERM, sig_term_handle);
        if (sigsetjmp(jmpbuf, 1)!=0 || sigsetjmp(jmpbuf, 2)!=0) //if SIGINT or SIGTERM signal caught, server close all sockets and exits
        {
            s->Closesockets();
            cout<<"Server exits, all sockets are closed"<<endl;
            exit(1);
        }
        bzero(&srvaddr, sizeof(srvaddr));
		bzero(&cliaddr, sizeof(cliaddr));
        rset = allset;
        if((Select(maxfd, &rset, NULL, NULL, NULL)) < 1)//server uses multi-threads to process different messages comes in, using select to poll different file descripters
            continue;
        if (FD_ISSET(sockfd_udp, &rset))
        {
			i = recvfrom(sockfd_udp, buf, MAXLINE, 0, (SA*)&cliaddr, &len);
            if (i < 0)
			{
				cerr<<"recv error"<<endl;
				continue;
			}
			else 
            {
                pthread_t tid;
                int port = ntohs(cliaddr.sin_port);
                if (port == ms->GetmsName().second) //when received notification from master, start handling master notifiction
                {
                    printf("%s\npush notification received from master\n", seperator);
                    ARGS args(s, buf, NULL, -1);
                    Pthread_create(&tid,NULL,&Proc_master_notification,(void *)&args);
                    Pthread_join(tid, NULL);
                    continue;
                }
                loss_frequency++;
                if (loss_frequency <= s->Getmessageloss() || s->Getmessageloss() == 0)
                {
                    msg_count++;
                    printf("No. %d message comes\n", msg_count);
                }
                else
                {
                    printf("Packet loss\n");
                    loss_frequency = 0;
                    continue;
                }
                if (msg_count > (s->Getlifetime()))
                {
                    printf("Life time expires, server exits\n");
                    s->Closesockets();
                    exit(1);
                }
                printf("Request from client: %s\n",Sock_ntop((SA*)&cliaddr,len));
                Request *req = new Request(buf);
                ARGS args(s, buf, req, -1);
                if (s->isTail())
                    Pthread_create(&tid,NULL,&Reply,(void *)&args);
                else
                    Pthread_create(&tid,NULL,&Sync,(void *)&args);
			}
        }
		if (FD_ISSET(sockfd_tcp, &rset))
		{
        	if((connfd = accept(sockfd_tcp, (SA*)&srvaddr, &len)) < 0)
        	{
            	cerr<<"accept error"<<endl;
        	}
        	else
        	{
                printf("Synchronization connection from previous server: %s\n",Sock_ntop((SA*)&srvaddr,len));
                i = read(connfd, buf, MAXLINE);
                if(i < 0)
                {
                    cout<<"read error"<<endl;
                }
                else
            	{
                    pthread_t tid;
                    Request *req = new Request(buf);
                    if (req->Getsynctype() == SyncSent) //when received sentTrans synchronization, start handling sentTrans synchronization
                    {
                        ARGS args(s, buf, req, -1);
                        Pthread_create(&tid, NULL, &Sync_Sent, (void *)&args);
                        Pthread_join(tid, NULL);
                        continue;
                    }
                    else if (req->Getsynctype() == SyncProc) //when received procTrans synchronization, start handling procTrans synchronization
                    {
                        ARGS args(s, buf, req, -1);
                        Pthread_create(&tid, NULL, &Sync_Hist, (void *)&args);
                        Pthread_join(tid, NULL);
                        continue;
                    }
                    msg_count++;
                    printf("No. %d message comes\n", msg_count);
                    if (msg_count > (s->Getlifetime()))
                    {
                        printf("Life time expires, server exits\n");
                        s->Closesockets();
                        exit(1);
                    }
                    ARGS args(s, buf, req, connfd);
                    if (s->isTail())
                        Pthread_create(&tid,NULL,&Reply,(void *)&args);
                    else
                        Pthread_create(&tid,NULL,&Sync,(void *)&args);
            	}
        	}
		}
    }
    return -1;
}

static void *Proc_master_notification(void *arg) //process master notification
{
    //Pthread_detach(pthread_self());
    struct ARGS *args = (struct ARGS *) arg;
    Server *s = args->srv;
    Push_Notification *noti = new Push_Notification(args->buf);
    s->Updatenext(noti->port_num);
    cout<<noti<<endl;
    if (s->isTail())
        //printf("I'm the new tail\n");
        return NULL;
    else if (noti->noti_type == New_Next)//when there is new next server set, start synchronize sentTrans and procTrans
    {
        Sync_sentTrans(s);
        Sync_procTrans(s);
    }
    else if (noti->noti_type == Extension)//when there is new server join(chain extension), start synchronize procTrans only
    {
        Sync_procTrans(s);
    }
}

static void Sync_sentTrans(Server *s) //synchronize every request in sentTrans to next server
{
    const int on = 1;
    int i;
    std::list <Request *>::iterator it;
    printf("%s\nNow starting synchronizing sentTrans to new next server\n", seperator);
    for (it = s->GetsentTrans().begin(); it != s->GetsentTrans().end(); ++it)
    {
        //sleep(1);
        cout<<(*it)<<endl;
        char sendbuf[MAXLINE];
        int sockfd = Socket(AF_INET, SOCK_STREAM, 0);
        Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        i = connect(sockfd, (SA*) & (s->Getnext()->Getsockaddr()), sizeof(s->Getnext()->Getsockaddr()));
        if (i < 0)
        {
            printf("synchronous sentTrans connect to next error\n");
            //printf("next is:%d\n", s->Getnext()->GetserverName().second);
            break;
        }
        (*it)->Packetize(sendbuf, SyncSent);
        i = write(sockfd, sendbuf, MAXLINE);
        if (i < 0)
        {
            printf("synchronous write to next error\n");
            break;
        }
    }
    printf("SentTrans synchronization to new next server finished\n%s\n", seperator);
}

static void Sync_procTrans(Server *s) //synchronize every request in procTrans to next server
{
    const int on = 1;
    int i;
    std::map <string, Request *>::iterator it;
    printf("%s\nNow starting synchronizing procTrans to new Tail\n", seperator);
    for (it = s->GetprocTrans().begin(); it != s->GetprocTrans().end(); ++it)
    {
        cout<<(*it).second<<endl;
        //sleep(1);
        char sendbuf[MAXLINE];
        int sockfd = Socket(AF_INET, SOCK_STREAM, 0);
        Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        i = connect(sockfd, (SA*) & (s->Getnext()->Getsockaddr()), sizeof(s->Getnext()->Getsockaddr()));
        if (i < 0)
        {
            printf("synchronous procTrans connect to next error\n");
            //printf("next is:%d\n", s->Getnext()->GetserverName().second);
            break;
        }
        (*it).second->Packetize(sendbuf, SyncProc);
        i = write(sockfd, sendbuf, MAXLINE);
        if (i < 0)
        {
            printf("synchronous write to next error\n");
        }
    }
    printf("ProcTrans synchronization to new Tail finished\n%s\n", seperator);
}

static void *Sync_Sent(void *arg) //when received sentTrans synchronization, process them
{
    const int on = 1;
    struct ARGS *args = (struct ARGS *) arg;
    Server *s = args->srv;
    class Reply *reply = new class Reply(args->req);
    printf("%s\nSentTrans synchronization received\n", seperator);
    cout<<args->req<<endl;
    printf("SentTrans synchronization finished\n%s\n", seperator);
    s->AddsentTrans(args->req);
    if (!s->isTail())
    {
        int i;
        int sockfd = Socket(AF_INET, SOCK_STREAM, 0);
        Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        i = connect(sockfd, (SA*) & (s->Getnext()->Getsockaddr()), sizeof(s->Getnext()->Getsockaddr()));
        if (i < 0)
        {
            printf("synchronous sentTrans connect to next error\n");
            //printf("next is:%d\n", s->Getnext()->GetserverName().second);
        }
        i = write(sockfd, args->buf, MAXLINE);
        if (i < 0)
        {
            printf("synchronous write to next error\n");
        }
    }
}

static void *Sync_Hist(void *arg) //when received procTrans synchronization, process them
{
    //Pthread_detach(pthread_self());
    struct ARGS *args = (struct ARGS *) arg;
    Server *s = args->srv;
    class Reply *reply = new class Reply(args->req);
    s->ProcReq(args->req, reply);
    printf("%s\nProcTrans synchronization received\n", seperator);
    cout<<args->req<<endl;
    cout<<reply<<endl;
    printf("ProcTrans synchronization finished\n%s\n", seperator);
    s->AddsentTrans(args->req);
    s->AckHist(args->req);
}

static void *Sync(void *arg) //process synchronization event, if synchroniation event comes, server uses an another tcp socket to connect the next server and then wait for the ack return
{
    int i;
    char recvbuf[MAXLINE];
    Pthread_detach(pthread_self());
	const int on = 1;
	int sockfd;
	struct ARGS *args = (struct ARGS *) arg;
	cout<<args->req<<endl;
	Server *s = args->srv;
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	i = connect(sockfd, (SA*) & (s->Getnext()->Getsockaddr()), sizeof(s->Getnext()->Getsockaddr()));
	if (i < 0)
    {
        printf("connect to next error\n");
        goto end;
    }
    i = write(sockfd, args->buf, MAXLINE);
    if (i < 0)
    {
        printf("write to next error\n");
        goto end;
    }
	s->AddsentTrans(args->req);
    if (readline(sockfd, recvbuf, MAXLINE) < 0)
        cerr<<"Read error"<<endl;
    else
    {
        if (!strlen(recvbuf))
            goto end;
        ACK *ack = new ACK(recvbuf);
		cout<<ack<<endl;
		if (args->connfd > 0)
    	{
        	i = write((args->connfd), recvbuf, MAXLINE);
        	if (i < 0)
            {
                printf("write to previous error\n");
                //cout<<"error "<<args->req<<endl;
            }
            close(args->connfd);
    	}
        class Reply *reply = new class Reply(args->req);
        s->ProcReq(args->req, reply);
        cout<<reply<<endl;
        s->AckHist(args->req);
    }
end:	close(sockfd);
}

static void *Reply(void *arg) //process the situation the server is the tail server and directly reply to client using its udp socket
{
    Pthread_detach(pthread_self());
    int i;
    struct ARGS *args = (struct ARGS *) arg;
	cout<<args->req<<endl;
    Server *s = args->srv;
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(struct sockaddr_in);
    ParseIpaddr(args->buf, cliaddr);
    char sendbuf[MAXLINE];
    
    class Reply *reply = new class Reply(args->req);
    s->ProcReq(args->req, reply);
    cout<<reply<<endl;
    reply->Packetize(sendbuf);
    
    Sendto(s->Getsockfd_udp(), sendbuf, MAXLINE, 0, (SA*)&cliaddr, len);
	s->AddsentTrans(args->req);
	s->AckHist(args->req);
    
	if (args->connfd > 0)
    {
		ACK *ack = new ACK(args->req);
		char buf[MAXLINE];
		ack->Packetize(buf);
        i = write((args->connfd), buf, MAXLINE);
        if (i < 0)
        {
            printf("write to previous error\n");
            //cout<<"error "<<args->req<<endl;
        }
        close(args->connfd);
    }
}

static void *Ping(void *arg)
{
    time_t cur_time;
    Pthread_detach(pthread_self());
    struct ARGS_PING *args = (struct ARGS_PING *) arg;
    socklen_t len = sizeof(struct sockaddr_in);
    Server *s = args->s;
    Master *ms = args->ms;
    while(1)
    {
        cur_time = time(NULL);
        //cout<<"current time is "<<cur_time<<endl;
        sleep(1);
        //cout<<"difference is "<<time(NULL) - cur_time<<endl;
        char msg[MAXLINE];
        s->packetize(msg);
        Sendto(s->Getsockfd_udp(), msg, MAXLINE, 0, (SA*)&(ms->Getsockaddr()), len);
    }
}

static void ParseIpaddr(string input_str, struct sockaddr_in &cliaddr) //parse the ipaddress inside client's request, regarding the situation the tail server receives the synchronization from previous and reply to client
{
    vector<string>vStr;
    char *input;
    int index = 0;
    boost::split(vStr, input_str, boost::is_any_of("|"), boost::token_compress_on);
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    for(vector<string>::iterator it = vStr.begin(); it != vStr.end(); ++it, ++index)
    {
        input = const_cast<char *>((*it).c_str());
        switch(index)
        {
            case 1:
                //cout<<input<<endl;
                Inet_pton(AF_INET, input, &cliaddr.sin_addr);
                break;
            case 2:
                //cout<<input<<endl;
                cliaddr.sin_port = htons(atoi(input));
                break;
            default:
                break;
        }
    }
}
