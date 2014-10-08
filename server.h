#include "inc.h"

#ifndef _SERVER_H__
#define _SERVER_H__

class Server
{
private:
	int sockfd_tcp;
    int sockfd_udp;
	int bankName;
	struct sockaddr_in srvaddr;
	string ip_addr;
	int port_num;	
	std::pair<string,int> sName;
    Server *next;
	std::vector<Request *> sentTrans;
	std::vector<Request *> procTrans;
	int startup_delay;
	int life_time;

public:
	Server():bankName(0),startup_delay(0),sName(make_pair("",-1)),next(NULL),life_time(0){}
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
        bzero(&srvaddr, sizeof(srvaddr));
        srvaddr.sin_family = AF_INET;
        Inet_pton(AF_INET, sName.first.c_str(), &srvaddr.sin_addr);
        srvaddr.sin_port = htons(sName.second);
    }
    void Setsocket()
    {
        const int on = 1;
        sockfd_tcp = Socket(AF_INET, SOCK_STREAM, 0);
        Setsockopt(sockfd_tcp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        Bind(sockfd_tcp, (SA*) & srvaddr, sizeof(srvaddr));
        sockfd_udp = Socket(AF_INET, SOCK_DGRAM, 0);
        Setsockopt(sockfd_udp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        Bind(sockfd_udp, (SA*) & srvaddr, sizeof(srvaddr));
    }
	void Setdelay(char *s)
	{
		startup_delay = atoi(s);
	}
	void Setlifetime(char *s)
	{
		life_time = atoi(s);
	}
	void Setnext(string input_str)
	{
        next = new Server();
        next->InitServ(input_str);
	}
    Server * Getnext()
    {
        return next;
    }
	int GetbankName()
	{
		return bankName;
	}
	pair<string,int> & GetserverName()
	{
		return sName;
	}
	struct sockaddr_in & Getsockaddr()
	{
		return srvaddr;
	}
	int Getsockfd_tcp()
	{
		return sockfd_tcp;
	}
    int Getsockfd_udp()
    {
        return sockfd_udp;
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
    /*void packetize (string &str)
>>>>>>> 7a861062dbe98953733f136174114e508710465a
    {
        str.append(prev.first);
        str.append(":");
        str.append(std::to_string(prev.second));
        str.append(seperator);
        str.append(next.first);
        str.append(":");
        str.append(std::to_string(next.second));
    }
    
    void depacketize(string str)
    {
        string ip_addr;
        int port_num;
        int index = 0;
        vector<string> vStr;
        tokenizer(str, vStr);
        char *input;
        for(vector<string>::iterator it = vStr.begin(); it != vStr.end(); ++it, ++index)
        {
            input = const_cast<char*>((*it).c_str());
            switch(index)
            {
                case 0:
                    ip_addr = input;
                    break;
                case 1:
                    port_num = atoi(input);
                    prev = make_pair(ip_addr,port_num);
                    break;
                case 2:
                    ip_addr = input;
                    break;
                case 3:
                    port_num = atoi(input);
                    next = make_pair(ip_addr, port_num);
                    break;
                default:
                    break;
            }
        }
    }*/
    
	friend ostream & operator << (ostream & cout, Server *s)
	{
		cout<<"bank name: "<<s->bankName<<endl;
		cout<<"server name: "<<s->sName.first<<":"<<s->sName.second<<endl;
		cout<<"start up delay: "<<s->startup_delay<<endl;
		cout<<"life time: "<<s->life_time<<endl;
        if (s->Getnext() != NULL)
            cout<<"next is: "<<s->next->GetserverName().first<<":"<<s->next->GetserverName().second<<endl;
		return cout;
	}
};
#endif
