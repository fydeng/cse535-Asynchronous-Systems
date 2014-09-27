#include "inc.h"
class Server
{
private:
	int bankName;
	string ip_addr;
	int port_num;	
	std::pair<string,int> sName;
	std::pair<string,int> prev;
	std::pair<string,int> next;
	std::vector<Request *> sentTrans;
	std::vector<Request *> procTrans;
	int startup_delay;
	int life_time;

public:
	Server():bankName(0),startup_delay(0),life_time(0),sName(make_pair("",-1)),prev(make_pair("",-1)),next(make_pair("",-1)){}
	void SetbankName(char *s)
	{
		bankName = atoi(s);
	}
	void Setipaddr(char *s)
	{
		ip_addr = s;
	}
	void Setportnum(char *s)
	{
		port_num = atoi(s);
		sName = make_pair (ip_addr, port_num);
	}
	void Setdelay(char *s)
	{
		startup_delay = atoi(s);
	}
	void Setlifetime(char *s)
	{
		life_time = atoi(s);
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
	void InitServ(char *input)
	{
		int index = 0;
		char *p = NULL;
		char *next_token = NULL;
		p = strtok_r(input, delim, &next_token);
		while(p)
		{
			//cout<<p<<endl;
			switch(index)
			{
				case 0:
					SetbankName(p);
					break;
				case 1:
					Setipaddr(p);
					break;
				case 2:
					Setportnum(p);
					break;
				case 3:
					Setdelay(p);
					break;
				case 4:
					Setlifetime(p);
				default:
					break;
			}
			p = strtok_r(NULL, delim, &next_token);
			index++;
		}
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
