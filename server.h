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
    std::map<int, float> Account_Info;
	std::list<Request *> sentTrans;
	std::map<string, Request *> procTrans;
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
        Setsockopt(sockfd_tcp,SOL_SOCKET,SO_DONTROUTE,&on,sizeof(on));
        Bind(sockfd_tcp, (SA*) & srvaddr, sizeof(srvaddr));
        sockfd_udp = Socket(AF_INET, SOCK_DGRAM, 0);
        Setsockopt(sockfd_udp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        Setsockopt(sockfd_udp,SOL_SOCKET,SO_DONTROUTE,&on,sizeof(on));
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
    int Getlifetime()
    {
        return life_time;
    }
	bool isTail()
	{
		if (next == NULL)
			return true;
		else 
			return false;
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
    
    int CheckHist(Request *req)
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
    
    void AddsentTrans(Request *req)
    {
        sentTrans.push_back(req);
		cout<<"Request "<<req->reqID<<" has been added to sentTranc"<<endl;
    }
    
    void AckHist(Request *req)
    {
        int result = CheckHist(req);
        for(list<Request *>::iterator it = sentTrans.begin(); it != sentTrans.end(); ++it)
        {
            if (((*it)->reqID == (req->reqID)) && ((*it)->reqtype == (req->reqtype)))
            {
                sentTrans.erase(it);
                if (!result)
                {
                    procTrans.insert(make_pair(req->reqID, req));
                    cout<<"Request "<<req->reqID<<" has been added to processed transaction"<<endl<<seperator<<endl;
                }
                else if(result == 1)
                    cout<<"Request "<<req->reqID<<" is inconsistent with history, do not add to processed transaction"<<endl;
                else
                    cout<<"Request "<<req->reqID<<" is a duplicate request, do not add to processed transaction"<<endl;
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
    
	void ProcReq(Request *req, Reply *reply)
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
    
    float Checkbal(int account_num)
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
    /*void packetize (string &str)
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
        if (!(s->isTail()))
            cout<<"next is: "<<s->next->GetserverName().first<<":"<<s->next->GetserverName().second<<endl;
		cout<<seperator<<endl;
		return cout;
	}
};
#endif
