#include "server.h"
#include "client.h"
class Master
{
	int sockfd_tcp;
	string ip_addr;
	int port;
	std::pair<string,int> mName;
	std::map<int, std::list<Server*> > sChain;
	std::map<int, std::list<Client*> > clients;

public:

	void Setipaddr(char *p)
	{
		ip_addr = p;
	}
	void Setportnum(char *p)
	{
		port = atoi(p);
		mName = make_pair(ip_addr, port);
    }
	pair<string, int> & GetmsName()
	{
		return mName;
	}
	int Getsockfd_tcp()
	{
		return sockfd_tcp;
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
	void Addserver(Server *s)
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
	void Addclient(Client *c)
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
	map<int, std::list<Server*> > & Getschain()
	{
		return sChain;
	}
	map<int, std::list<Client*> > & Getclients()
	{
		return clients;
	}
    Server * Search_Head_Server(int bankname)
    {
        map<int, list<Server*> >::iterator it = sChain.find(bankname);
        if (it == sChain.end())
        {
            cout<<"did not find the server"<<endl;
            return NULL;
        }
        return it->second.front();
    }
    Server * Search_Server(InitReq *req, string ip_addr, int port_num)
    {
        map<int, list<Server*> >::iterator it = sChain.find(req->bankName);
        if (it == sChain.end())
        {
            cout<<"did not find the server"<<endl;
            return NULL;
        }
        for(list<Server *>::iterator it1 = (it->second).begin(); it1 != (it->second).end(); ++it1)
        {
            if (((*it1)->GetserverName().first == ip_addr) && ((*it1)->GetserverName().second == port_num))
                return (*it1);
        }
        return NULL;
    }
	friend ostream & operator << (ostream & cout, Master &ms)
	{
		int chainnum = 1;
		cout<<"------------------Master---------------------"<<endl;
		cout<<"Master name is: "<<ms.ip_addr<<":"<<ms.port<<endl;
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
