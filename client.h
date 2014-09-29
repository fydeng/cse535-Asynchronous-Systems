#include "inc.h"

#ifndef _CLI_H__
#define _CLI_H__

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
	friend ostream & operator << (ostream & cout, Client *c)
	{
		cout<<"bank name: "<<c->bankName<<endl;
		cout<<"account no.:"<<c->account_no<<endl;
		cout<<"server name: "<<c->cName.first<<":"<<c->cName.second<<endl;
		return cout;
	}
};
#endif
