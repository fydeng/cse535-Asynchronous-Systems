#include "inc.h"

#ifndef _CLI_H__
#define _CLI_H__

bool retrans;
int retrans_inteval;
int retrans_time;
bool random_req;
float seed;
int num_req;
float prob_getbalance;
float prob_deposit;
float prob_withdraw;
float prob_transfer;

void parse_config(string input_str) //parse configuration file to get configuration of clients
{
	char *input;
	vector<string> vStr;
	tokenizer(input_str,vStr);
	int index = 0;
	cout<<"----------------Clients Configuration---------------------"<<endl;
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
            case 3:
                if(atoi(input))
                {
                    random_req = true;
                    cout<<"randomized request"<<endl;
                }
                else
                {
                    random_req = false;
                    cout<<"itemized request"<<endl<<endl;
                }
                break;
			default:
				break;
		}
	}
}

void parse_randomized_req(string input_str) //if request is randomized, parse the configuration
{
    char *input;
    vector<string> vStr;
    tokenizer(input_str,vStr);
    int index = 0;
    cout<<"----------------Randomized Request Configuration---------------------"<<endl;
    for(vector<string>::iterator it = vStr.begin(); it != vStr.end(); ++it, ++index)
    {
        input = const_cast<char *>((*it).c_str());
        if (!strlen(input))
        {
            index --;
            continue;
        }
        switch(index)
        {
            case 0:
                seed = atof(input);
                cout<<"Seed of random request is: "<<seed<<endl;
                break;
            case 1:
                num_req = atoi(input);
                cout<<"number of request for each client is: "<<num_req<<endl;
                break;
            case 2:
                prob_getbalance = atof(input);
                cout<<"probability of get balance is: "<<prob_getbalance<<endl;
                break;
            case 3:
                prob_deposit = atof(input);
                cout<<"probability of deposit is: "<<prob_deposit<<endl;
                break;
            case 4:
                prob_withdraw = atof(input);
                cout<<"probability of withdraw is: "<<prob_withdraw<<endl;
                break;
            case 5:
                prob_transfer = atof(input);
                cout<<"probability of transfer is: "<<prob_transfer<<endl;
                break;
            default:
                break;
        }
    }
}

vector <Request *> req_list;
class Client  //class of client
{
private:	//private members of client
	struct sockaddr_in cliaddr; //the struct of client socket address
	int sockfd_udp; //the socket of client, which is an udp socket
	int bankName; //the bankname of client
	int account_no; //the account of client
	string ip_addr; //the ip address of client
	int port_num; //the port number of client
	std::pair<string, int> cName; //client name, which is a tuple containing ip address and port number
    std::list<Request*> rlist; //client's request list

public:
    Client():bankName(0),account_no(0),ip_addr(""),port_num(0),cName(make_pair("",-1)){} //constructor
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
	void Setportnum(char *p) //set port number
	{
		port_num = atoi(p);
		cName = make_pair(ip_addr, port_num);
		bzero(&cliaddr, sizeof(cliaddr));
		cliaddr.sin_family = AF_INET;
		Inet_pton(AF_INET, cName.first.c_str(), &cliaddr.sin_addr);
		cliaddr.sin_port = htons(cName.second);
	}
	void Setsocket() //set and bind the socket
	{
		const int on = 1;
		sockfd_udp = Socket(AF_INET, SOCK_DGRAM, 0);
        Setsockopt(sockfd_udp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		Setsockopt(sockfd_udp,SOL_SOCKET,SO_DONTROUTE,&on,sizeof(on));
        Bind(sockfd_udp, (SA*) & cliaddr, sizeof(cliaddr));
	}
    void Addrequest(Request *req)
    {
        rlist.push_back(req);
    }
	void Packetize(Request *req, char *buf) //packetize the request to send to server
	{
        string str;
		string str1;
		stringstream sstream;
		str = (req->req_str);
        sstream << "|";
		sstream << ip_addr;
		sstream << "|";
		sstream << port_num;
        sstream >> str1;
        str.append(str1);
        strcpy(buf, str.c_str());
	}
	int GetbankName()
	{
		return bankName;
	}
	int Getsocket()
	{
		return sockfd_udp;
	}
    int GetAccountno()
    {
        return account_no;
    }
    struct sockaddr_in & Getsockaddr()
    {
        return cliaddr;
    }
    list<Request*> & GetReqList()
    {
        return rlist;
    }
    pair<string,int> GetclientName()
    {
        return cName;
    }
	void InitCli(string input_str) //initialize the client
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
    void GenerateRandomReq(int client_index) //generate random request for each client
    {
        float ran_num;
        enum ReqType reqtype;
        srand(seed);
        float amount = 0;
        for(int i = 0; i < num_req; i++)
        {
            ran_num = (float)((float)(rand() % 100) / 100);
            //cout<<"random number is "<<ran_num<<endl;
            if(ran_num < prob_getbalance)
                reqtype = Query;
            else if (ran_num < (prob_deposit + prob_getbalance))
                reqtype = Deposit;
            else if (ran_num < (prob_deposit + prob_getbalance + prob_withdraw))
                reqtype = Withdraw;
            else
                reqtype = Transfer;
            if (reqtype == Query)
                amount = 0;
            else
                amount = (ran_num) * 1000;
            Request *req = new Request(reqtype, bankName, client_index, i+1, account_no, amount);
            cout<<req<<endl;
            rlist.push_back(req);
        }
    }
    
	friend ostream & operator << (ostream & cout, Client *c)
	{
        printf("bank name: %d\naccount no.: %d\nclient name: %s:%d\n", c->bankName, c->account_no, c->cName.first.c_str(), c->cName.second);
        //printf("account no.: %d\n", c->account_no);
        //printf("client name: %s:%d\n", c->cName.first.c_str(), c->cName.second);
		return cout;
	}
};
#endif
