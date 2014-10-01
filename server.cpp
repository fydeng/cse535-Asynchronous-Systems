extern "C"
{
#include "unpthread.h"
}
#include "server.h"
#include "master.h"

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in srvaddr, cliaddr;
	ifstream fin;
	fin.open("config.txt");
	string input_str;
	char *input;
	Server *s = new Server();
	Master *ms = new Master();
	int index = atoi(argv[1]);
	int i = 0;
	int flag_server = 0, flag_master = 0;
	const int on = 1;
	while(fin.good())
	{
		getline(fin, input_str);
		if (input_str == "\0")
			continue;
     	input = const_cast<char*>(input_str.c_str());		
		if (!strncmp(input, "//Server", 8))
		{
			flag_server = 1;
			continue;
		}
		else if (!strncmp(input, "//Master", 8))
		{
			flag_master = 1;
			continue;
		}
		if (flag_master)
		{
			ms->Init(input);
			flag_master = 0;
			continue;
		}
		if (flag_server && ((++i) == index))
		{
			s->InitServ(input);
			break;
		}
	}
	cout<<s;
//	cout<<"master name is: "<<ms->GetmsName().first<<":"<<ms->GetmsName().second<<endl;
	cout<<ms;
	s->Setsocket();
	sockfd = s->Getsockfd_tcp();
	Connect(sockfd, (SA*) &(ms->Getsockaddr()), sizeof(ms->Getsockaddr()));
	string str = "hello world";
	char buf[MAXLINE];
	int n;
	if((write(sockfd, str.c_str(), strlen(str.c_str()) + 1)) < 0)
		cout<<"write error"<<endl;
	if((n = read(sockfd, buf, MAXLINE)) > 0)
	{
		cout<<buf<<endl;
	}
}
