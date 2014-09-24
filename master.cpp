#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <list>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;

char delim[] = " ;,()";

class Server
{
private:
	int bankName;
	string ip_addr;
	int port_num;	
	std::pair<string,int> sName;
	std::pair<string,int> prev;
	std::pair<string,int> next;
	int startup_delay;
	int life_time;

public:
	Server():bankName(0),startup_delay(0),life_time(0),sName(make_pair("",-1)),prev(make_pair("",-1)),next(make_pair("",-1)){}
	void SetbankName(string str)
	{
		bankName = atoi(str.c_str());
	}
	void Setipaddr(string str)
	{
		ip_addr = str;
	}
	void Setportnum(string str)
	{
		port_num = atoi(str.c_str());
		sName = make_pair (ip_addr, port_num);
	}
	void Setdelay(string str)
	{
		startup_delay = atoi(str.c_str());
	}
	void Setlifetime(string str)
	{
		life_time = atoi(str.c_str());
	}
	void Setprev(Server *s)
	{
		prev = make_pair(s->GetserverName().first, s->GetserverName().second);
	}
	void Setnext(Server *s)
	{
		next = make_pair(s->GetserverName().first, s->GetserverName().second);
	}
	int GetbankName()
	{
		return bankName;
	}
	pair<string,int> & GetserverName()
	{
		return sName;
	}
	friend ostream & operator << (ostream & cout, Server *s)
	{
		cout<<"bank name: "<<s->bankName<<endl;
		cout<<"ip address: "<<s->sName.first<<endl;
		cout<<"port number: "<<s->sName.second<<endl;
		cout<<"start up delay: "<<s->startup_delay<<endl;
		cout<<"life time: "<<s->life_time<<endl;
		cout<<"predecessor is: "<<s->prev.first<<" "<<s->prev.second<<endl;
		cout<<"next is: "<<s->next.first<<" "<<s->next.second<<endl;
		return cout;
	}
};


class Master
{
private:
	std::pair<string,int> mName;
	std::map<int, std::list<Server*> > sChain;
	//std::map<int, std::list<Client*> > clients;

public:
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
	map<int, std::list<Server*> > & Getschain()
	{
		return sChain;
	}	
};

int main()
{
	ifstream fin;
	fin.open("config.txt");
	char *input;	
	string input_str;
	vector<string>vStr;
	int tmp_bank = 1;
	Master ms;	
	while(fin.good())
	{	
		int index = 0;	
		getline(fin, input_str);
		if (input_str == "\0")
			continue;
     		input = const_cast<char*>(input_str.c_str());		
		if (strncmp(input, "//", 2) == 0)
			continue;
		Server *s = new Server();
		boost::split(vStr, input_str, boost::is_any_of(delim), boost::token_compress_on );
		for(vector<string>::iterator it = vStr.begin(); it != vStr.end(); ++ it,++index)
		{
			//cout << *it << endl;
			switch(index)
			{
				case 0:
					s->SetbankName(*it);
					break;
				case 1:
					s->Setipaddr(*it);
					break;
				case 2:
					s->Setportnum(*it);
					break;
				case 3:
					s->Setdelay(*it);
					break;
				case 4:
					s->Setlifetime(*it);
				default:
					break;
			}												
		}
		ms.Addserver(s);
	}
	for(map<int, list<Server*> >::iterator it1 = ms.Getschain().begin(); it1 != ms.Getschain().end(); ++it1)
	{
		int count = 0;
		for(list<Server *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
		{
			list<Server *>::iterator cur = it2;
			if(cur == it1->second.begin())
			{
				cout<<"front"<<endl;
				(*cur)->Setnext(*(++it2));
			}
			else if(count == it1->second.size() - 1)
			{
				cout<<"back"<<endl;
				(*cur)->Setprev(*(--it2));
			}
			else
			{
				(*cur)->Setnext(*(++it2));
				it2 = cur;
				(*cur)->Setprev(*(--it2));
			}
			it2 = cur;
			cout<<(*it2)<<endl;
		}
	}
	system("pause");
	return 1;
}
