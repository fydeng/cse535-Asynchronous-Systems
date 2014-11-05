extern "C"
{
#include "unpthread.h"
}
#include "master.h"

static sigjmp_buf jmpbuf;
static void sig_int_handle(int signo) //handle the signal of SIGINT
{
    siglongjmp(jmpbuf, 1);
}
static void sig_term_handle(int signo) //handle the signal of SIGTERM
{
    siglongjmp(jmpbuf, 2);
}

int failure_inteval;

static void *handle_ping(void*);
static void *probing(void *);
static void depacketize(char *, pair<int, int> &);

Master ms;
int main(int argc, char **argv)
{
	ifstream fin;
	fin.open(argv[1]);
	ms.InitMS(fin);
	cout<<ms;
    ms.Setsocket();
    int sockfd_tcp, sockfd_udp, maxfd;
    struct sockaddr_in srvaddr;
    fd_set allset, rset;
    socklen_t len = sizeof(struct sockaddr_in);
    sockfd_tcp = ms.Getsockfd_tcp();
    sockfd_udp = ms.Getsockfd_udp();
    maxfd = max(sockfd_tcp, sockfd_udp) + 1;
    FD_ZERO(&allset);
    FD_SET(sockfd_tcp, &allset);
    FD_SET(sockfd_udp, &allset);
    char buf[MAXLINE];
    int i;
    pthread_t tid_probe;
    Pthread_create(&tid_probe, NULL, &probing, NULL);
    while(1)
    {
        Signal(SIGINT, sig_int_handle);
        Signal(SIGTERM, sig_term_handle);
        if (sigsetjmp(jmpbuf, 1)!=0 || sigsetjmp(jmpbuf, 2)!=0) //if SIGINT or SIGTERM signal caught, server close all sockets and exits
        {
            ms.Closesockets();
            cout<<"Master exits, all sockets are closed"<<endl;
            exit(1);
        }
        rset = allset;
        if ((Select(maxfd, &rset, NULL, NULL, NULL)) < 1)
            continue;
        if (FD_ISSET(sockfd_udp, &rset))
        {
            i = recvfrom(sockfd_udp, buf, MAXLINE, 0, (SA*)&srvaddr, &len);
            if (i < 0)
            {
                cerr<<"recv error"<<endl;
                continue;
            }
            else
            {
                pthread_t tid;
                Pthread_create(&tid, NULL, &handle_ping, (void *)buf);
            }
        }
    }
	return 1;
}

static void *handle_ping(void *arg)
{
    Pthread_detach(pthread_self());
    char *buf = (char *)arg;
    pair <int, int> srv_name;
    depacketize(buf, srv_name);
    //printf("%s\n",seperator);
    //printf("Ping received: 127.0.0.1:%d\n",srv_name.second);
    ms.update_time_sheet(srv_name);
    //ms.display_time_sheet();
    if (!ms.Server_exists(srv_name))
    {
        printf("New server joined:127.0.0.1:%d, chain extension start\n",srv_name.second);
        Server *s = new Server(srv_name);
        ms.Addserver(s);
        ms.client_notify(srv_name, true);
        ms.server_notify(srv_name, true);
    }
}

static void *probing(void *arg)
{
    Pthread_detach(pthread_self());
    while (1)
    {
        sleep(5);
        printf("start probing\n");
        time_t cur_time = time(NULL);
        for(map<pair<int, int>, time_t >::iterator it = ms.Gettimesheet().begin(); it != ms.Gettimesheet().end(); ++it)
        {
            if (cur_time - (it->second) > 5)
            {
                printf("%s\n",seperator);
                printf("Server 127.0.0.1:%d FAILS\n",it->first.second);
                ms.client_notify(it->first, false);
                ms.server_notify(it->first, false);
                it = ms.Gettimesheet().erase(it);
                if (it == ms.Gettimesheet().end())
                    break;
            }
        }
    }
}

void Master::server_notify(pair <int, int> srv, bool extension)
{
    Server *prev = Search_Prev_Server(srv, !extension);
    cal_next(srv.first);
    if (prev == NULL)
    {
        printf("Head server fails, the successor automatically becomes the new head\n");
        return;
    }
    else
    {
        
        //printf("previous server is %d\n", prev->GetserverName().second);
        socklen_t len = sizeof(sockaddr_in);
        char send_msg[MAXLINE];
        Server *next = Search_Next_Server(prev);
        if (next == NULL)
        {
            srv.second = -1;
        }
        else
        {
            srv.second = next->GetserverName().second;
        }
        packetize(send_msg, srv, false, extension);
        Sendto(sockfd_udp, send_msg, MAXLINE, 0, (SA*)&(prev->Getsockaddr()), len);
        if (extension)
            printf("New server extended to the tail of chain %d\n%s\n", srv.first, seperator);
        else if (srv.second == -1)
            printf("Tail server fails, server notification sent out to its precessor: 127.0.0.1:%d\n", prev->GetserverName().second);
        else
            printf("Internal server fails, server notification sent out to its precessor: 127.0.0.1:%d, new next server is 127.0.0.1:%d\n", prev->GetserverName().second, srv.second);
    }
}

void Master::client_notify(pair <int, int> srv, bool extension)
{
    char send_msg[MAXLINE];
    socklen_t len = sizeof(struct sockaddr_in);
    ms.packetize(send_msg, srv, !extension, extension);
    //printf("%s\n",send_msg);
    map<int, list<Client*> >::iterator it1 = clients.find(srv.first);
    for(list<Client *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
    {
        Sendto(sockfd_udp, send_msg, MAXLINE, 0, (SA*)&((*it2)->Getsockaddr()), len);
        if (extension)
            printf("Push notification of chain extension to all clients sent\n");
        else
            printf("Push notification of server fail to all clients sent\n");
    }
}

static void depacketize(char *buf, pair<int, int> &sname)
{
    char *input;
    vector<string> vStr;
    tokenizer(buf,vStr);
    int index = 0;
    for(vector<string>::iterator it = vStr.begin(); it != vStr.end(); ++it, ++index)
    {
        input = const_cast<char *>((*it).c_str());
        switch(index)
        {
            case 0:
                sname.first = atoi(input);
                break;
            case 1:
                sname.second = atoi(input);
                break;
            default:
                break;
        }
    }
}

void Master::InitMS(ifstream &fin)
{
    char *input;
    string input_str;
    int flag_master = 0, flag_server = 0, flag_client = 0;
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
        else if (!strncmp(input, "//Client ", 9))
        {
            flag_server = 0;
            continue;
        }
        else if (!strncmp(input, "//Client:", 9))
        {
            flag_client = 1;
            continue;
        }
        else if (!strncmp(input, "//Request", 9))
            break;
        if (flag_master)
        {
            Init(input_str);
            flag_master = 0;
            continue;
        }
        if (flag_server)
        {
            Server *s = new Server();
            s->InitServ(input_str);
            if (!s->Getdelay())
                Addserver(s);
        }
        if (flag_client)
        {
            Client *c = new Client();
            c->InitCli(input_str);
            Addclient(c);
        }
    }
    for(map<int, list<Server*> >::iterator it1 = sChain.begin(); it1 != sChain.end(); ++it1)
    {
        int count = 0;
        for(list<Server *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
        {
            list<Server *>::iterator cur = it2;
            if(count == it1->second.size() - 1)
            {
                continue;
            }
            else
            {
                (*cur)->Setnext(*(++it2));
                
            }
            it2 = cur;
        }
    }
};