#include "server.h"

int main()
{
	ifstream fin;
	fin.open("config.txt");
	string input_str;
	char *input;
	Server *s = new Server();
	int index = 2;
	int i = 0;
	while(fin.good())
	{
		getline(fin, input_str);
		if (input_str == "\0")
			continue;
     	input = const_cast<char*>(input_str.c_str());		
		if (strncmp(input, "//", 2) == 0)
			continue;
		if ((++i) == index)
		{
			s->InitServ(input);
			break;
		}
	}
	cout<<s;
}
