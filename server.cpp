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
	int connfd;
};

static void *Sync(void *arg);

int main(int argc, char **argv)
{
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
	pthread_t tid;
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
	cout<<s;
	cout<<ms;
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
			i = recvfrom(sockfd_udp, buf, MAXLINE, 0, (SA*)&cliaddr, &len);			    if (i < 0) 
			{
				cout<<"recv error"<<endl;
				continue;
			}
			else 
			{
				cout<<buf<<endl;
            	cout<<"receive from "<<Sock_ntop((SA*)&cliaddr,len)<<endl;
				struct ARGS args;
				args.srv = s;
				args.buf = buf;
				Pthread_create(&tid,NULL,&Sync,(void *)&args);
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
            	printf("connection from %s\n", Sock_ntop((SA*)&srvaddr,len));
            	if((i = read(connfd, buf, MAXLINE))>0)
            	{
                	cout<<buf<<endl;
            	}
				struct ARGS args;
				args.srv = s;
				args.buf = buf;
				args.connfd = connfd;
				Pthread_create(&tid,NULL,&Sync,(void *)&args);
				close(connfd);
        	}
		}
    }
    return -1;
}

static void *Sync(void *arg)
{
	const int on = 1;
	int sockfd;
	struct ARGS *args = (struct ARGS *) arg;
	Server *s = args->srv;
	Pthread_detach(pthread_self());				
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	Connect(sockfd, (SA*) & (s->Getnext()->Getsockaddr()), sizeof(s->Getnext()->Getsockaddr()));
	Write(sockfd, args->buf, MAXLINE);
	close(sockfd);
}
