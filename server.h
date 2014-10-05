//#include "inc.h"

#ifndef _SERVER_H__
#define _SERVER_H__

class Server
{
private:
	int sockfd_tcp;
	int bankName;
//	struct sockaddr_in srvaddr;
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
	Server():bankName(0),startup_delay(0),sName(make_pair("",-1)),prev(make_pair("",-1)),next(make_pair("",-1)),life_time(0){}
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
//		bzero(&srvaddr, sizeof(srvaddr));
//		srvaddr.sin_family = AF_INET;
//		Inet_pton(AF_INET, sName.first.c_str(), &srvaddr.sin_addr);
//		srvaddr.sin_port = htons(sName.second);
	}
/*	void Setsocket()
	{
		const int on = 1;
		sockfd_tcp = Socket(AF_INET, SOCK_STREAM, 0);
		Setsockopt(sockfd_tcp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		Bind(sockfd_tcp, (SA*) & srvaddr, sizeof(srvaddr));
	}*/
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
	int Getsockfd_tcp()
	{
		return sockfd_tcp;
	}
	void InitServ(string input_str)
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
					SetbankName(input);
					break;
				case 1:
					Setipaddr(input);
					break;
				case 2:
					Setportnum(input);
					break;
				case 3:
					Setdelay(input);
					break;
				case 4:
					Setlifetime(input);
				default:
					break;
			}
		}
	}
	friend ostream & operator << (ostream & cout, Server *s)
	{
		cout<<"bank name: "<<s->bankName<<endl;
		cout<<"server name: "<<s->sName.first<<":"<<s->sName.second<<endl;
		cout<<"start up delay: "<<s->startup_delay<<endl;
		cout<<"life time: "<<s->life_time<<endl;
		cout<<"predecessor is: "<<s->prev.first<<":"<<s->prev.second<<endl;
		cout<<"next is: "<<s->next.first<<":"<<s->next.second<<endl;
		return cout;
	}
};
#endif
