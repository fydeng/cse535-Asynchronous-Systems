#include "master.h"

void Master::InitMS(ifstream &fin)
{
	char *input;
	string input_str;
	while(fin.good())
	{
		getline(fin, input_str);
		if (input_str == "\0")
			continue;
     		input = const_cast<char*>(input_str.c_str());		
		if (strncmp(input, "//", 2) == 0)
			continue;
		Server *s = new Server();
		s->InitServ(input);
		Addserver(s);
	}
	for(map<int, list<Server*> >::iterator it1 = Getschain().begin(); it1 != Getschain().end(); ++it1)
	{
		int count = 0;
		for(list<Server *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
		{
			list<Server *>::iterator cur = it2;
			if(cur == it1->second.begin())
			{
				(*cur)->Setnext(*(++it2));
			}
			else if(count == it1->second.size() - 1)
			{
				(*cur)->Setprev(*(--it2));
			}
			else
			{
				(*cur)->Setnext(*(++it2));
				it2 = cur;
				(*cur)->Setprev(*(--it2));
			}
			it2 = cur;
		}
	}
};

/*int main()
{
	ifstream fin;
	fin.open("config.txt");
	Master ms;
	ms.InitMS(fin);
	cout<<ms;
	system("pause");
	return 1;
}*/
