#include "client.h"

int main()
{
	ifstream fin;
	fin.open("config.txt");
	string input_str;
	char *input;
	Client *c = new Client();
	int flag_client = 0;
	while(fin.good())
	{
		getline(fin, input_str);
		if (input_str == "\0")
			continue;
		input = const_cast<char*>(input_str.c_str());
		if (strncmp(input, "//Client", 8) == 0)
		{
			flag_client = 1;
			continue;
		}
		if (flag_client)
		{
			c->InitCli(input);
			cout<<c;
		}
	}
}
