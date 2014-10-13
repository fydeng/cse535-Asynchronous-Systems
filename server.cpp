extern "C"
{
#include "unpthread.h"
}
#include "server.h"
#include "master.h"

struct ARGS
{
    Server *srv;
    char *buf;
    Request *req;
    class Reply *reply;
    int connfd;
    
    ARGS(Server *s, char *buffer, Request *request, Reply *rep, int conn_fd)
    {
        srv = s;
        buf = buffer;
        req = request;
        reply = rep;
        connfd = conn_fd;
    }
};

static void *Sync(void *);
static void ParseIpaddr(string input_str, struct sockaddr_in &cliaddr);
static void *Reply(void *);

int main(int argc, char **argv)
{
    //FLAGS_logtostderr = 1;
    FLAGS_logtostderr = 0;
    FLAGS_log_dir = "./";
    google::InitGoogleLogging(argv[0]);
    struct sockaddr_in srvaddr, cliaddr;
    int sockfd_tcp, sockfd_udp, connfd, maxfd;
	ifstream fin;
	fin.open("config.txt");
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
    int msg_count = 0;
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
                s->Setnext(input_str);
            break;
        }
	}
	cout<<"----------------Configuration information of current server---------------"<<endl;
	cout<<s;
	//cout<<ms;
    s->Setsocket();
    sockfd_tcp = s->Getsockfd_tcp();
    sockfd_udp = s->Getsockfd_udp();
    listen(sockfd_tcp, LISTENQ);
    maxfd = max(sockfd_tcp,sockfd_udp) + 1;
    FD_ZERO(&allset);
    FD_SET(sockfd_tcp, &allset);
    FD_SET(sockfd_udp, &allset);
    while(1)
    {
        bzero(&srvaddr, sizeof(srvaddr));
		bzero(&cliaddr, sizeof(cliaddr));
        rset = allset;
        if((Select(maxfd, &rset, NULL, NULL, NULL)) < 1)
            continue;
        if (FD_ISSET(sockfd_udp, &rset))
        {
			i = recvfrom(sockfd_udp, buf, MAXLINE, 0, (SA*)&cliaddr, &len);
            if (i < 0)
			{
				cout<<"recv error"<<endl;
				continue;
			}
			else 
            {
                msg_count++;
                cout<<"No. "<<msg_count<<" message comes"<<endl;
                if (msg_count == s->Getlifetime())
                {
                    cout<<"Number of messages equals the life time, server exits"<<endl;
                    exit(1);
                }
                cout<<"Request from client: "<<Sock_ntop((SA*)&cliaddr,len)<<endl;
				pthread_t tid;
                void *status;
                Request *req = new Request(buf);
                class Reply *reply = new class Reply(req);
				s->ProcReq(req, reply);
                ARGS args(s, buf, req, reply, -1);
                if (s->isTail())
                    Pthread_create(&tid,NULL,&Reply,(void *)&args);
                else
                    Pthread_create(&tid,NULL,&Sync,(void *)&args);
                //Pthread_join(tid, &status);
			}
        }
		if (FD_ISSET(sockfd_tcp, &rset))
		{
        	if((connfd = accept(sockfd_tcp, (SA*)&srvaddr, &len)) < 0)
        	{
            	cout<<"accept error"<<endl;
        	}
        	else
        	{
                msg_count++;
                cout<<"No. "<<msg_count<<" message comes"<<endl;
                if (msg_count == s->Getlifetime())
                {
                    cout<<"Number of messages equals the life time, server exits"<<endl;
                    exit(1);
                }
                cout<<"Syncronization connection from previous server: "<<Sock_ntop((SA*)&srvaddr,len)<<endl;
                i = read(connfd, buf, MAXLINE);
                if(i < 0)
                {
                    cout<<"read error"<<endl;
                }
                else
            	{
                    pthread_t tid;
                    void *status;
                    Request *req = new Request(buf);
                    class Reply *reply = new class Reply(req);
                    s->ProcReq(req, reply);
                    ARGS args(s, buf, req, reply, connfd);
                    if (s->isTail())
                        Pthread_create(&tid,NULL,&Reply,(void *)&args);
                    else
                        Pthread_create(&tid,NULL,&Sync,(void *)&args);
                    //Pthread_join(tid, &status);
            	}
        	}
		}
    }
    return -1;
}

static void *Sync(void *arg)
{
    char recvbuf[MAXLINE];
    Pthread_detach(pthread_self());
	const int on = 1;
	int sockfd;
	struct ARGS *args = (struct ARGS *) arg;
	cout<<args->req<<endl;
	cout<<args->reply<<endl;
	Server *s = args->srv;
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	Connect(sockfd, (SA*) & (s->Getnext()->Getsockaddr()), sizeof(s->Getnext()->Getsockaddr()));
	Write(sockfd, args->buf, MAXLINE);
	s->AddsentTrans(args->req);
    if (readline(sockfd, recvbuf, MAXLINE) < 0)
        cout<<"Read error"<<endl;
    else
    {
        ACK *ack = new ACK(recvbuf);
		cout<<ack<<endl;
		if (args->connfd > 0)
    	{
        	Write(args->connfd, recvbuf, MAXLINE);
        	close(args->connfd);
    	}
        s->AckHist(args->req);
    }
	close(sockfd);
}

static void *Reply(void *arg)
{
    Pthread_detach(pthread_self());
    struct ARGS *args = (struct ARGS *) arg;
	cout<<args->req<<endl;
	cout<<args->reply<<endl;
    Server *s = args->srv;
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(struct sockaddr_in);
    ParseIpaddr(args->buf, cliaddr);
    char sendbuf[MAXLINE];
    (args->reply)->Packetize(sendbuf);
    Sendto(s->Getsockfd_udp(), sendbuf, MAXLINE, 0, (SA*)&cliaddr, len);
	s->AddsentTrans(args->req);
	s->AckHist(args->req);
	if (args->connfd > 0)
    {
		ACK *ack = new ACK(args->req);
		char buf[MAXLINE];
		ack->Packetize(buf);
        Write(args->connfd, buf, MAXLINE);
        close(args->connfd);
    }
}

static void ParseIpaddr(string input_str, struct sockaddr_in &cliaddr)
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
