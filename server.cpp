#include "server.h"
#include "master.h"

int main(int argc, char **argv)
{
    using namespace boost::asio;
	int sockfd;
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
			s->InitServ(input_str);
			break;
		}
	}
	cout<<s;
	cout<<ms;
/*	s->Setsocket();
	sockfd = s->Getsockfd_tcp();
	Connect(sockfd, (SA*) &(ms->Getsockaddr()), sizeof(ms->Getsockaddr()));
	string str = "hello world";
//	struct InitReq req;
//	memset(&req, 0, sizeof(struct InitReq));
//	req.src = server;
	//req.ip_addr = const_cast<char*>(s->GetserverName().first.c_str());
//	strcpy(req.ip_addr, s->GetserverName().first.c_str());
//	cout<<req.ip_addr<<" "<<sizeof(req.ip_addr)<<endl;
	req.port_num = s->GetserverName().second;
	req.bankName = s->GetbankName();
	char buf[MAXLINE];
	int n;
	void *send_buf = (void*)malloc(sizeof(struct InitReq));
	memset(send_buf, 0, sizeof(struct InitReq));
	memcpy(send_buf, &req, sizeof(struct InitReq));

	if((write(sockfd, send_buf, sizeof(send_buf))) < 0)
		cout<<"write error"<<endl;
	if((n = read(sockfd, buf, MAXLINE)) > 0)
	{
		cout<<buf<<endl;
	}*/
	io_service iosev;
	ip::tcp::socket socket(iosev);
	ip::tcp::endpoint ep(ip::address_v4::from_string(ms->GetmsName().first), s->GetmsName().second);
	boost::system::error_code ec;
	socket.connect(ep,ec);
	if(ec)
	{
		std::cout << boost::system::system_error(ec).what()<<std::endl;
		return -1;
	}
	char buf[100];
	size_t len = socket.read_some(buffer(buf),ec);
	std::cout.write(buf,len);
}
