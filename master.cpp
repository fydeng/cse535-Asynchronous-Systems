extern "C"
{
	#include "unpthread.h"
}
#include "master.h"

void Master::InitMS(ifstream &fin)
{
	char *input;
	string input_str;
	int flag_master = 0, flag_server = 0;
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
		{
			break;
		}

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
			Addserver(s);
		}
	}
	for(map<int, list<Server*> >::iterator it1 = Getschain().begin(); it1 != Getschain().end(); ++it1)
{
		int count = 0;
		for(list<Server *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
		{
			list<Server *>::iterator cur = it2;
			if(cur == it1->second.begin())
			{
				(*cur)->Setnext(*(++it2));
			}
			else if(count == it1->second.size() - 1)
			{
				(*cur)->Setprev(*(--it2));
			}
			else
			{
				(*cur)->Setnext(*(++it2));
				it2 = cur;
				(*cur)->Setprev(*(--it2));
			}
			it2 = cur;
		}
	}
};

int main()
{
	struct sockaddr_in srvaddr, cliaddr;
	int listen_fd, connfd, maxfd, nready;
	const int on = 1;
	fd_set allset, rset;
	socklen_t len;
	ifstream fin;
	fin.open("config.txt");
	Master ms;
	ms.InitMS(fin);
	cout<<ms;

	ms.Setsocket();
	listen_fd = ms.Getsockfd_tcp();
	listen(listen_fd, LISTENQ);
	maxfd = listen_fd + 1;
	FD_ZERO(&allset);
	FD_SET(listen_fd, &allset);
	while(1)
	{
		bzero(&srvaddr, sizeof(srvaddr));
		rset = allset;
		if((select(maxfd, &rset, NULL, NULL, NULL)) < 1)
			continue;
		if((connfd = accept(listen_fd, (SA*)&srvaddr, &len)) < 0)
		{
			cout<<"accept error"<<endl;
		}
		else
		{	
			char str_srv[INET_ADDRSTRLEN];
		//	Inet_ntop(AF_INET, &srvaddr.sin_addr, str_srv, sizeof(str_srv));
		//	cout<<"connection from "<<Sock_ntop((SA*)&srvaddr,len)<<endl;
		//	cout<<"connection from "<<str_srv<<endl;
			printf("connection from %s\n", Sock_ntop((SA*)&srvaddr,len));
			int n;
			char buf[MAXLINE];
			if((n=read(connfd, buf, MAXLINE))>0)
			{
				InitReq *req = (InitReq *)buf;
				if(req->src==server)
					cout<<"server request"<<endl;
			}
			if((n=write(connfd, "hello", 6))<0)
				cout<<"write error"<<endl;
		}
		close(connfd);
	}
//	system("pause");
	return 1;
}
