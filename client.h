#include "inc.h"

#ifndef _CLI_H__
#define _CLI_H__

bool retrans;
int retrans_inteval;
int retrans_time;

void parse_config(char *input)
{
	int index = 0;
	char *p = NULL;
	char *next_token = NULL;
	p = strtok_r(input, delim, &next_token);
	while(p)
	{
		switch(index)
		{
			case 0:
				if (atoi(p))
					retrans = true;
				else
					retrans = false;
				break;
			case 1:
				retrans_inteval = atoi(p);
				break;
			case 2:
				retrans_time = atoi(p);
				break;
			default:
				break;
		}
		p = strtok_r(NULL, delim, &next_token);
		index++;
	}
}

class Client
{
private:	
	int bankName;
	int account_no;
	string ip_addr;
	int port_num;
	std::pair<string, int> cName;
	vector<Request *>req_list;

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
	void InitCli(char *input)
	{
		int index = 0;
		char *p = NULL;
		char *next_token = NULL;
		p = strtok_r(input, delim, &next_token);
		while(p)
		{
			switch(index)
			{
				case 0:
					SetbankName(p);
					break;
				case 1:
					SetAccountNo(p);
					break;
				case 2:
					Setipaddr(p);
					break;
				case 3:
					Setportnum(p);
					break;
				default:
					break;
			}
			p = strtok_r(NULL, delim, &next_token);
			index++;
		}
	}
	void Initreq(char *input)
	{
		int index = 0;
		char *p = NULL;
		char *next_token = NULL;
		p = strtok_r(input, delim, &next_token);
		Request *req = new Request();
		while(p)
		{
			switch(index)
			{
				case 0:
					if (!strcmp(p,"getBalance"))
						req->reqtype = Query;
					else if (!strcmp(p, "deposit"))
						req->reqtype = Deposit;
					else if (!strcmp(p, "withdraw"))
						req->reqtype = Withdraw;
					else if (!strcmp(p, "Transfer"))
						req->reqtype = Transfer;
					break;
				case 1:
					req->reqID = p;
					break;
				case 2:
					req->account_num = atoi(p);
					break;
				case 3:
					req->amount = atoi(p);
					break;
				default:
					break;
			}
			p = strtok_r(NULL, delim, &next_token);
			index++;
		}
		req_list.push_back(req);
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
