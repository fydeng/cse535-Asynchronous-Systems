#include "client.h"
#include "master.h"

int main()
{
	ifstream fin;
	fin.open("config.txt");
	string input_str;
	char *input;
	Master *ms = new Master();
	int flag_master = 0, flag_client = 0, flag_config = 0, flag_request = 0;
	while(fin.good())
	{
		getline(fin, input_str);
		if (input_str == "\0")
			continue;
		input = const_cast<char*>(input_str.c_str());

		if (strncmp(input, "//Master", 8) == 0)
		{
			flag_master = 1;
			continue;
		}
		else if (strncmp(input, "//Client ", 9) == 0)
		{
			flag_config = 1;
			continue;
		}
		else if (strncmp(input, "//Client:", 9) == 0)
		{
			flag_client = 1;
			continue;
		}
		else if (strncmp(input, "//Request", 9) == 0)
		{
			flag_request = 1;
			flag_client = 0;
			continue;
		}
		if (flag_master)
		{
			ms->Init(input_str);
			flag_master = 0;
			continue;
		}
		if (flag_config)
		{
			parse_config(input_str);
			flag_config = 0;
/*			if (retrans)
				cout<<"retransmission"<<endl;
			else
				cout<<"no retransmission"<<endl;
			cout<<"retransmission inteval is "<<retrans_inteval<<endl;
			cout<<"retransmission time is "<<retrans_time<<endl;*/
			continue;
		}
		if (flag_client)
		{
			Client *c = new Client();
			c->InitCli(input_str);
			cout<<c;
			client_list.push_back(c);
		}
		if (flag_request)
		{
			Parsereq(input_str);
		}
	}
	cout<<ms;
	for(vector<Request*>::iterator it = req_list.begin();it!=req_list.end();++it)
	{
		if((*it)->reqtype == Query)
			cout<<"getBalance ";
		else if((*it)->reqtype == Deposit)
			cout<<"Deposit ";
		else if((*it)->reqtype == Withdraw)
			cout<<"Withdraw ";
		else if((*it)->reqtype == Transfer)
			cout<<"Transfer "<<endl;
		cout<<"req Id: "<<(*it)->reqID<<" account_num "<<(*it)->account_num<<" amount "<<(*it)->amount<<endl;
	}
}
