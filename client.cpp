extern "C"
{
#include "unpthread.h"
}
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
			ms->Init(input);
			flag_master = 0;
			continue;
		}
		if (flag_config)
		{
			parse_config(input);
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
			c->InitCli(input);
			cout<<c;
		}
		if (flag_request)
		{
			
		}
	}
	cout<<ms;
}
