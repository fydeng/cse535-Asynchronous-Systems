#include "inc.h"

#ifndef _SERVER_H__
#define _SERVER_H__

class Server //class of server
{
private:
	int sockfd_tcp; //each server has two sockets: tcp socket and udp socket
    int sockfd_udp;
	int bankName;
	struct sockaddr_in srvaddr; //server's address struct
	string ip_addr;
	int port_num;	
	std::pair<string,int> sName;
    Server *next;
    std::map<int, float> Account_Info; //server's account info stores the information of clients' account information
	std::list<Request *> sentTrans; //server's sent transaction list
	std::map<string, Request *> procTrans; //server's processed transaction list
	int startup_delay; //start-up delay of server
	int life_time; //life time of server

public:
	Server():bankName(0),startup_delay(0),sName(make_pair("",-1)),next(NULL),life_time(0){}
    Server(pair<int, int> srv)
    {
        bankName = srv.first;
        ip_addr = "127.0.0.1";
        Setportnum(srv.second);
        next = NULL;
        startup_delay = 0;
        life_time = 0;
    }
	void SetbankName(int bn)
    {
        bankName = bn;
    }
    void SetbankName(char *s)
	{
		bankName = atoi(s);
	}
	void Setipaddr(char *s)
	{
		ip_addr = s;
	}
    void Setportnum(int port)
    {
        port_num = port;
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
        Setsockopt(sockfd_tcp,SOL_SOCKET,SO_DONTROUTE,&on,sizeof(on));
        Bind(sockfd_tcp, (SA*) & srvaddr, sizeof(srvaddr));
        sockfd_udp = Socket(AF_INET, SOCK_DGRAM, 0);
        Setsockopt(sockfd_udp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        Setsockopt(sockfd_udp,SOL_SOCKET,SO_DONTROUTE,&on,sizeof(on));
        Bind(sockfd_udp, (SA*) & srvaddr, sizeof(srvaddr));
    }
    void Closesockets()
    {
        close(sockfd_tcp);
        close(sockfd_udp);
    }
	void Setdelay(char *s)
	{
		startup_delay = atoi(s);
	}
	void Setlifetime(char *s)
	{
		life_time = atoi(s);
	}
    void Updatenext(int port_num)
    {
        if (port_num == -1)
        {
            next = NULL;
            return;
        }
        if (next)
        {
            next->Setportnum(port_num);
        }
        else
        {
            next = new Server();
            next->SetbankName(bankName);
            next->Setipaddr("127.0.0.1");
            next->Setportnum(port_num);
        }
    }
    void Setnext(Server *s)
    {
        next = s;
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
    int Getlifetime()
    {
        return life_time;
    }
    int Getdelay()
    {
        return startup_delay;
    }
    map <string, Request *> & GetprocTrans()
    {
        return procTrans;
    }
	bool isTail()
	{
		if (next == NULL)
			return true;
		else 
			return false;
	}
	void InitServ(string input_str) //init for server
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
					Setportnum(atoi(input));
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
    
    int CheckHist(Request *req) //check server's processed transaction
    {
//		DisplayprocTrans();
        std::map<string, Request *>::iterator it;
        it=procTrans.find(req->reqID);
        if(it!=procTrans.end())
        {
            if ((req->reqtype) == (it->second->reqtype))
                return 2;
            else
                return 1;
        }
        else
            return 0;
    }
    
    void AddsentTrans(Request *req) //add request to server's sent transaction
    {
        if (req->reqtype == Query)
        {
            printf("Query do not add to sent transaction\n%s\n", seperator);
            return;
        }
        
        sentTrans.push_back(req);
        printf("Request %s has been added to sent transaction\n%s\n", req->reqID.c_str(), seperator);
    }
    
    void AckHist(Request *req) //add request to server's processed transaction
    {
        if (req->reqtype == Query)
        {
            printf("Query do not add to processed transaction\n%s\n", seperator);
            return;
        }
        int count_clear = 0;
        for(list<Request *>::iterator it = sentTrans.begin(); it != sentTrans.end(); ++it)
        {
            if (((*it)->reqID == (req->reqID)) && ((*it)->reqtype == (req->reqtype)))
            {
                sentTrans.erase(it);
                if (count_clear != 0)
                    printf("Using ACK to clear old Request %s\n", (*it)->reqID.c_str());
                count_clear++;
                int result = CheckHist(req);
                if (!result)
                {
                    procTrans.insert(make_pair(req->reqID, req));
                    printf("Request %s has been added to processed transaction\n%s\n", req->reqID.c_str(), seperator);
                }
                else if(result == 1)
                    printf("Request %s is inconsistent with history, do not add to processed transaction\n%s\n", req->reqID.c_str(), seperator);
                else
                    printf("Request %s is a duplicate request, do not add to processed transaction\n%s\n", req->reqID.c_str(), seperator);
            }
        }
    }
   
    void DisplaysentTrans()
	{
		cout<<seperator<<endl;
		for(list<Request *>::iterator it = sentTrans.begin(); it != sentTrans.end(); ++it)
			cout<<(*it)->reqID<<" ";
		cout<<seperator<<endl;
	}

	void DisplayprocTrans()
	{
		cout<<seperator<<endl;
		for(map<string, Request *>::iterator it = procTrans.begin(); it != procTrans.end(); ++it)
			cout<<it->second<<endl;
		cout<<seperator<<endl;
	}
    
	void ProcReq(Request *req, Reply *reply) //process transaction for client's request
    {
        int result = CheckHist(req);
        float cur_bal = Checkbal(req->account_num);
        float new_bal = 0;
        if (result)
		{
			reply->balance = cur_bal;
            if (result > 1)
                reply->outcome = Processed;
            else
                reply->outcome = InconsistentWithHistory;
			return;
		}
        else
        {
            if (req->reqtype == Query)
            {
                reply->balance = cur_bal;
                reply->outcome = Processed;
            }
            else if (req->reqtype == Deposit)
            {
                new_bal = cur_bal + req->amount;
                Account_Info[req->account_num] = new_bal;
                reply->balance = new_bal;
                reply->outcome = Processed;
            }
            else if (req->reqtype == Withdraw)
            {
                new_bal = cur_bal - (req->amount);
                if (new_bal < 0)
                {
                    reply->outcome = InsufficientFunds;
                    new_bal = cur_bal;
                }
                else
                {
                    reply->outcome = Processed;
                    Account_Info[req->account_num] = new_bal;
                }
                reply->balance = new_bal;
            }
        }
    }
    
    void packetize(char *msg)
    {
        string str = "";
        str = std::to_string(bankName);
        str.append(":");
        str.append(std::to_string(sName.second));
        strcpy(msg, str.c_str());
    }
    
    float Checkbal(int account_num) //check balance
    {
        std::map<int, float>::iterator it;
        it=Account_Info.find(account_num);
        if(it!=Account_Info.end())
        {
            return it->second;
        }
        else
        {
            Account_Info.insert(make_pair(account_num, 0));
            return 0;
        }
    }
    
	friend ostream & operator << (ostream & cout, Server *s)
	{
		cout<<"bank name: "<<s->bankName<<endl;
		cout<<"server name: "<<s->sName.first<<":"<<s->sName.second<<endl;
		cout<<"start up delay: "<<s->startup_delay<<endl;
		cout<<"life time: "<<s->life_time<<endl;
        if (!(s->isTail()))
            cout<<"next is: "<<s->next->GetserverName().first<<":"<<s->next->GetserverName().second<<endl;
		cout<<seperator<<endl;
		return cout;
	}
};
#endif
