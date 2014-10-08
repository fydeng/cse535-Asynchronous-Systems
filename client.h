#include "inc.h"

#ifndef _CLI_H__
#define _CLI_H__

bool retrans;
int retrans_inteval;
int retrans_time;

void parse_config(string input_str)
{
	char *input;
	vector<string> vStr;
	tokenizer(input_str,vStr);
	int index = 0;
	for(vector<string>::iterator it = vStr.begin(); it != vStr.end(); ++it, ++index)	
	{
		input = const_cast<char *>((*it).c_str());
		switch(index)
		{
			case 0:
				if (atoi(input))
				{
					retrans = true;
					cout<<"retransmission enabled"<<endl;
				}
				else
					retrans = false;
				break;
			case 1:
				retrans_inteval = atoi(input);
				cout<<"retransmission interval is: "<<retrans_inteval<<endl;
				break;
			case 2:
				retrans_time = atoi(input);
				cout<<"retransmission time is: "<<retrans_time<<endl;
				break;
			default:
				break;
		}
	}
}

vector <Request *> req_list;
class Client
{
private:	
	struct sockaddr_in cliaddr;
	int sockfd_udp;
	int bankName;
	int account_no;
	string ip_addr;
	int port_num;
	std::pair<string, int> cName;

public:
    Client():bankName(0),account_no(0),ip_addr(""),port_num(0),cName(make_pair("",-1)){}
    Client(int bankname, string ipaddr, int portnum)
    {
        bankName = bankname;
        account_no = 0;
        ip_addr = ipaddr;
        port_num = portnum;
        cName = make_pair(ip_addr, port_num);
    }
	void SetbankName(char *p)
	{
		bankName = atoi(p);
	}
	void SetAccountNo(char *p)
	{
		account_no = atoi(p);
	}
	void Setipaddr(char *p)
	{
		ip_addr = p;
	}
	void Setportnum(char *p)
	{
		port_num = atoi(p);
		cName = make_pair(ip_addr, port_num);
		bzero(&cliaddr, sizeof(cliaddr));
		cliaddr.sin_family = AF_INET;
		Inet_pton(AF_INET, cName.first.c_str(), &cliaddr.sin_addr);
		cliaddr.sin_port = htons(cName.second);
	}
	void Setsocket()
	{
		const int on = 1;
		sockfd_udp = Socket(AF_INET, SOCK_DGRAM, 0);
        	Setsockopt(sockfd_udp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		Setsockopt(sockfd_udp,SOL_SOCKET,SO_DONTROUTE,&on,sizeof(on));
        	Bind(sockfd_udp, (SA*) & cliaddr, sizeof(cliaddr));
	}
	void Packetize(Request *req, string &send_str)
	{
		string str;
		stringstream sstream;
		send_str = req->req_str;
		sstream << ip_addr;
		sstream << "|";
		sstream << port_num;
		sstream >> str;
		send_str.append("|");
		send_str.append(str);
	}
	int GetbankName()
	{
		return bankName;
	}
	int Getsocket()
	{
		return sockfd_udp;
	}
    pair<string,int> GetclientName()
    {
        return cName;
    }
	void InitCli(string input_str)
	{
		char *input;
		vector<string> vStr;
		tokenizer(input_str,vStr);
		int index = 0;
		for(vector<string>::iterator it = vStr.begin(); it != vStr.end(); ++it, ++index)	
		{
			input = const_cast<char *>((*it).c_str());
			switch(index)
			{
				case 0:
					SetbankName(input);
					break;
				case 1:
					SetAccountNo(input);
					break;
				case 2:
					Setipaddr(input);
					break;
				case 3:
					Setportnum(input);
					break;
				default:
					break;
			}
		}
	}

	friend ostream & operator << (ostream & cout, Client *c)
	{
		cout<<"bank name: "<<c->bankName<<endl;
		cout<<"account no.:"<<c->account_no<<endl;
		cout<<"client name: "<<c->cName.first<<":"<<c->cName.second<<endl;
		return cout;
	}
};
#endif
