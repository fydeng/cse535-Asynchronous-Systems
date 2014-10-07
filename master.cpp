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
	using namespace boost::asio;
	ifstream fin;
	fin.open("config.txt");
	Master ms;
	ms.InitMS(fin);
	cout<<ms;
	io_service iosev;
	ip::tcp::acceptor acceptor(iosev, ip::tcp::endpoint(ip::address_v4::from_string(ms.GetmsName().first), ms.GetmsName().second));
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	for(;;)
	{
        char buf[100];
        string send_str;
        ip::udp::endpoint local_udp(ip::address_v4::from_string(ms.GetmsName().first), ms.GetmsName().second);
        boost::system::error_code error;
		ip::tcp::socket socket_tcp(iosev);
        ip::udp::socket socket_udp(iosev);
        socket_udp.open(ip::udp::v4());
        socket_udp.bind(local_udp);
        ip::udp::endpoint udp_ep;
        socket_udp.receive_from(boost::asio::buffer(buf),udp_ep, 0, error);
        cout<<buf<<endl;
        InitReq *req = new InitReq();
        req->depacketize(buf);
        if (req->src == client)
        {
            cout<<"client come"<<endl;
            string ip_addr = udp_ep.address().to_string();
            int port = udp_ep.port();
            cout<<ip_addr<<":"<<port<<endl;
            Client *cli = new Client(req->bankName, ip_addr, port);
            ms.Addclient(cli);
            Server *srv;
            srv = ms.Search_Head_Server(cli->GetbankName());
            srv->packetize(send_str);
            
        }
        if (error && error != boost::asio::error::message_size)
            throw boost::system::system_error(error);
        
        //send_str = "hello i am master";
        boost::system::error_code ignored_error;
        socket_udp.send_to(boost::asio::buffer(send_str),
                       udp_ep, 0, ignored_error);
		/*acceptor.accept(socket_tcp);
        boost::system::error_code ec;
        socket.read_some(buffer(buf), ec);
        string ip_addr = socket.remote_endpoint().address().to_string();
        int port = socket.remote_endpoint().port();
        cout<<buf<<endl;
        InitReq *req = new InitReq();
        req->depacketize(buf);
        if (req->src == server)
        {
            cout<<"server come"<<endl;
            Server *srv;
            srv = ms.Search_Server(req, ip_addr, port);
            cout<<srv<<endl;
            srv->packetize(send_str);
        }
        else if (req->src == client)
            cout<<"client come"<<endl;
        cout<<ip_addr<<":"<<port<<endl;
        socket.write_some(buffer(send_str),ec);
        if(ec)
        {
			std::cout <<boost::system::system_error(ec).what() << std::endl;
            break;
        }*/
	}

	return 1;
}
