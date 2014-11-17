#include "server.h"
#include "client.h"

class Master //class of master
{
	int sockfd_tcp; //each master has two sockets: tcp socket and udp socket, tcp socket is used for communicating with server
    int sockfd_udp; //udp socket is used for communicating with client
    struct sockaddr_in msaddr; //struct socket address of master
	string ip_addr; //master's ip address
	int port; //master's port number
	std::pair<string,int> mName; //master name
	std::map<int, std::list<Server*> > sChain; //master stores all server topology
	std::map<int, std::list<Client*> > clients; //master stores all clients
    std::map<pair<int, int>, time_t> time_sheet; //master stores a time sheet of server, in order to record the newest ping of server, correspoding to its time, and check whether certain server fails

public:

	void Setipaddr(char *p) //some basic functions for initialization, similar to server
	{
		ip_addr = p;
	}
	void Setportnum(char *p)
	{
		port = atoi(p);
		mName = make_pair(ip_addr, port);
        bzero(&msaddr, sizeof(msaddr));
        msaddr.sin_family = AF_INET;
        Inet_pton(AF_INET, mName.first.c_str(), &msaddr.sin_addr);
        msaddr.sin_port = htons(mName.second);
    }
    void Setsocket()
    {
        const int on = 1;
        sockfd_tcp = Socket(AF_INET, SOCK_STREAM, 0);
        Setsockopt(sockfd_tcp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        Setsockopt(sockfd_tcp,SOL_SOCKET,SO_DONTROUTE,&on,sizeof(on));
        Bind(sockfd_tcp, (SA*) & msaddr, sizeof(msaddr));
        sockfd_udp = Socket(AF_INET, SOCK_DGRAM, 0);
        Setsockopt(sockfd_udp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        Setsockopt(sockfd_udp,SOL_SOCKET,SO_DONTROUTE,&on,sizeof(on));
        Bind(sockfd_udp, (SA*) & msaddr, sizeof(msaddr));
    }
    void Closesockets()
    {
        close(sockfd_tcp);
        close(sockfd_udp);
    }
	pair<string, int> & GetmsName()
	{
		return mName;
	}
	int Getsockfd_tcp()
	{
		return sockfd_tcp;
	}
    int Getsockfd_udp()
    {
        return sockfd_udp;
    }
    map<pair<int, int>, time_t> & Gettimesheet()
    {
        return time_sheet;
    }
    struct sockaddr_in & Getsockaddr()
    {
        return msaddr;
    }
	void Init(string input_str)
	{
		char *input;
		int index = 0;
		vector<string> vStr;
		tokenizer(input_str, vStr);
		for(vector<string>::iterator it = vStr.begin(); it != vStr.end(); ++it, ++index)
		{
			input = const_cast<char*>((*it).c_str());
			switch(index)
			{
				case 0:
					Setipaddr(input);
					break;
				case 1:
					Setportnum(input);
					break;
				default:
					break;
			}
		}
	}
	void InitMS(ifstream &fin);
	void Addserver(Server *s) //add server to its server chain
	{
		int bankname = s->GetbankName();		
		std::map<int, std::list<Server*> >::iterator it;
		it=sChain.find(bankname);
		if(it!=sChain.end())
		{
			it->second.push_back(s);
		}
		else
		{
			std::list<Server*> serverchain;
			serverchain.push_back(s);
			sChain.insert(std::pair<int,std::list<Server*> >(bankname,serverchain));
		}
	}
	void Addclient(Client *c) //add client to its client chain
	{
		int bankname = c->GetbankName();		
		std::map<int, std::list<Client*> >::iterator it;
		it=clients.find(bankname);
		if(it!=clients.end())
		{
			it->second.push_back(c);
		}
		else
		{
			std::list<Client*> clientchain;
			clientchain.push_back(c);
			clients.insert(std::pair<int,std::list<Client*> >(bankname,clientchain));
		}
    }
    void update_time_sheet(pair<int,int> srv) //every time master receives ping from server, it updates the time sheet
    {
        time_t cur_time = time(NULL);
        for(map<pair<int, int>, time_t >::iterator it = time_sheet.begin(); it != time_sheet.end(); ++it)
        {
            if (it->first.first == srv.first && it->first.second == srv.second)
            {
                it->second = cur_time;
                return;
            }
        }
        time_sheet.insert(std::pair<std::pair<int, int>,time_t>(srv,cur_time));
    }
    void display_time_sheet()
    {
        for(map<pair<int, int>, time_t >::iterator it = time_sheet.begin(); it != time_sheet.end(); ++it)
        {
            cout<<it->first.first<<":"<<it->first.second<<" "<<it->second<<endl;
        }
    }
	map<int, std::list<Server*> > & Getschain()
	{
		return sChain;
	}
	map<int, std::list<Client*> > & Getclients()
	{
		return clients;
	}
    Server * Search_Head_Server(int bankname) //search for head server in the server chain
    {
        map<int, list<Server*> >::iterator it = sChain.find(bankname);
        if (it == sChain.end())
        {
            cout<<"did not find the server"<<endl;
            return NULL;
        }
        return it->second.front();
    }
    bool Server_exists(pair<int, int> srv) //check whether certain server exists
    {
        map<int, list<Server*> >::iterator it = sChain.find(srv.first);
        if (it == sChain.end())
        {
            return false;
        }
        for(list<Server *>::iterator it1 = (it->second).begin(); it1 != (it->second).end(); ++it1)
        {
            if (((*it1)->GetserverName().second == srv.second))
            {
                return true;
            }
        }
        return false;
    }
    Server * Search_Prev_Server(pair<int, int> srv, bool del) //search for the previous server of certain server
    {
        map<int, list<Server*> >::iterator it = sChain.find(srv.first);
        if (it == sChain.end())
        {
            return NULL;
        }
        for(list<Server *>::iterator it1 = (it->second).begin(); it1 != (it->second).end(); ++it1)
        {
            if (((*it1)->GetserverName().second == srv.second))
            {
                if (del)
                    it1 = it->second.erase(it1);
                if (it1 == it->second.begin())
                    return NULL;
                else
                    return (*(--it1));
            }
        }
        return NULL;
    }
    Server * Search_Next_Server(Server *s) //search for the next server of certain server
    {
        map<int, list<Server*> >::iterator it = sChain.find(s->GetbankName());
        if (it == sChain.end())
        {
            return NULL;
        }
        for(list<Server *>::iterator it1 = (it->second).begin(); it1 != (it->second).end(); ++it1)
        {
            if (((*it1)->GetserverName().second == s->GetserverName().second))
            {
                return (*it1)->Getnext();
            }
        }
        return NULL;
    }
    void packetize(char *send_msg, pair<int, int> srv, bool fail, bool extend)
    {
        int type;
        string str = "";
        if (fail)
            type = 0;
        else if (extend)
            type = 1;
        else
            type = 2;
        str = std::to_string(type);
        str.append(":");
        str.append(std::to_string(srv.first));
        str.append(":");
        str.append(std::to_string(srv.second));
        //printf("%s\n",str.c_str());
        strcpy(send_msg, str.c_str());
    }
    void server_notify(pair <int, int> srv, bool extension); //notify server of server fail/chain extension
    void client_notify(pair <int, int> srv, bool extension); //notify client of server fail/chain extension
    void cal_next(int bankname)
    {
        map<int, list<Server*> >::iterator it1 = sChain.find(bankname);
        int count = 0;
        for(list<Server *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
        {
            list<Server *>::iterator cur = it2;
            if(count == it1->second.size() - 1)
            {
                (*cur)->Setnext(NULL);
                continue;
            }
            else
            {
                (*cur)->Setnext(*(++it2));
                
            }
            it2 = cur;
        }
        display_server_chain();
    }
    void display_server_chain()
    {
        int chainnum = 1;
        cout<<"------------------Server Chain---------------------"<<endl;
        for(map<int, list<Server*> >::iterator it1 = sChain.begin(); it1 != sChain.end(); ++it1, ++chainnum)
        {
            cout<<"-----------------Chain "<<chainnum<<"---------------------"<<endl;
            int count = 0;
            for(list<Server *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
            {
                if(it2 == it1->second.begin())
                {
                    cout<<"Head"<<endl;
                }
                else if(count == it1->second.size() - 1)
                {
                    cout<<"Tail"<<endl;
                }
                cout<<(*it2)<<endl;
            }
        }
    }
	friend ostream & operator << (ostream & cout, Master &ms)
	{
		int chainnum = 1;
		cout<<"------------------Master---------------------"<<endl;
		cout<<"Master name is: "<<ms.ip_addr<<":"<<ms.port<<endl<<endl;
        cout<<"------------------Server Chain---------------------"<<endl;
		for(map<int, list<Server*> >::iterator it1 = ms.Getschain().begin(); it1 != ms.Getschain().end(); ++it1, ++chainnum)
		{
			cout<<"-----------------Chain "<<chainnum<<"---------------------"<<endl;
			int count = 0;
			for(list<Server *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
			{
				if(it2 == it1->second.begin())
				{
					cout<<"Head"<<endl;
				}
				else if(count == it1->second.size() - 1)
				{
					cout<<"Tail"<<endl;
				}
				cout<<(*it2)<<endl;
			}
		}
        cout<<"------------------Client Chain---------------------"<<endl;
		for(map<int, list<Client*> >::iterator it3 = ms.Getclients().begin(); it3 != ms.Getclients().end(); ++it3, ++chainnum)
		{
			cout<<"-----------------Clients in bank "<<it3->first<<"---------------------"<<endl;
			int count = 0;
			for(list<Client *>::iterator it4 = it3->second.begin(); it4 != it3->second.end(); ++it4,++count)
			{
				cout<<(*it4)<<endl;
			}
		}
		return cout;
	}
	friend ostream & operator << (ostream & cout, Master *ms)
	{
		cout<<"Master name is: "<<ms->GetmsName().first<<":"<<ms->GetmsName().second<<endl;
		return cout;
	}
};
