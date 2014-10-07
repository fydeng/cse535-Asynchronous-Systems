#include "server.h"
#include "master.h"

int main(int argc, char **argv)
{
    int sockfd;
	ifstream fin;
	fin.open("config.txt");
	string input_str;
	char *input;
	Master *ms = new Master();
    Server *s = new Server();
    Server *next = new Server();
	int index = atoi(argv[1]);
	int i = 0;
	int flag_server = 0, flag_next = 0, flag_master = 0;
	const int on = 1;
	while(fin.good())
	{
		getline(fin, input_str);
		if (input_str == "\0")
			continue;
     	input = const_cast<char*>(input_str.c_str());
		if (!strncmp(input, "//Master", 8))
		{
			flag_master = 1;
			continue;
		}
        else if (!strncmp(input, "//Server", 8))
        {
            flag_server = 1;
            continue;
        }
        else if (!strncmp(input, "//Client", 8))
            break;
        if (flag_master)
		{
			ms->Init(input);
			flag_master = 0;
			continue;
		}
		else if (flag_server && ((++i) == index))
		{
			s->InitServ(input_str);
            flag_server = 0;
            flag_next = 1;
            continue;
		}
        else if (flag_next)
        {
            next->InitServ(input_str);
            break;
        }
	}
	cout<<s;
    cout<<next;
	cout<<ms;
    return -1;
}
