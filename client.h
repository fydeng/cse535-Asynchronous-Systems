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

vector<Request *> req_list;

void Parsereq(string input_str)
{
	char *input;
	vector<string> vStr;
	tokenizer(input_str,vStr);
	int index = 0;
//	Request *req = new Request();
	struct Request *req = new Request;
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
				if (!strncmp(input,"getBalance",10))
				{
					req->reqtype = Query;
				}
				else if (!strcmp(input, "deposit"))
					req->reqtype = Deposit;
				else if (!strcmp(input, "withdraw"))
					req->reqtype = Withdraw;
				else if (!strcmp(input, "Transfer"))				
					req->reqtype = Transfer;
				break;
			case 1:
				req->reqID = input;
				break;
			case 2:
				req->account_num = atoi(input);
				break;
			case 3:
				req->amount = atoi(input);
				break;
			default:
				break;
		}
	}
	req_list.push_back(req);
}

class Client
{
private:	
	int bankName;
	int account_no;
	string ip_addr;
	int port_num;
	std::pair<string, int> cName;

public:
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
	}
	int GetbankName()
	{
		return bankName;
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

vector<Client*> client_list;
#endif
