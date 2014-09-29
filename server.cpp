#include "server.h"
#include "master.h"

int main()
{
	int sockfd;
	struct sockaddr_in msaddr, srvaddr, cliaddr;
	ifstream fin;
	fin.open("config.txt");
	string input_str;
	char *input;
	Server *s = new Server();
	int index = 2;
	int i = 0;
	int flag_server = 0;
	while(fin.good())
	{
		getline(fin, input_str);
		if (input_str == "\0")
			continue;
     	input = const_cast<char*>(input_str.c_str());		
		if (strncmp(input, "//Server", 8) == 0)
		{
			flag_server = 1;
			continue;
		}
		if (flag_server && ((++i) == index))
		{
			s->InitServ(input);
			break;
		}
	}
	cout<<s;
	cout<<"master name is: "<<msName.first<<":"<<msName.second<<endl;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&srvaddr, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	inet_pton(AF_INET, msName.first.c_str(), &msaddr.sin_addr);
	srvaddr.sin_port = htons(s->GetserverName().second);
	if((connect(sockfd, (SA*) &msaddr, sizeof(msaddr))) < 0)
	{
		cout<<"error connection"<<endl;
	}
	string str = "hello world";
	if((write(sockfd, str.c_str(), strlen(str.c_str()) + 1)) < 0)
		cout<<"write error"<<endl;
}
