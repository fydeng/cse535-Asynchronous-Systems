extern "C"
{
#include "unpthread.h"
}
#include "client.h"
#include "master.h"
#include "server.h"

std::map<int, std::vector<Client*> >cChain;
std::map<int, std::list<Server*> > sChain;

void Addserver(Server *s);
void Addclient(Client *c);
static Client * SearchClient(Request *);
static Server * SearchServer(Request *);
void displaychain();

int main()
{
	ifstream fin;
	fin.open("config.txt");
	string input_str;
	char *input;
	Master *ms = new Master();
	int flag_master = 0, flag_client = 0, flag_config = 0, flag_request = 0, flag_server = 0;
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
            flag_master = 0;
            flag_server = 1;
            continue;
        }
		else if (!strncmp(input, "//Client ", 9))
		{
            flag_server = 0;
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
		}
        else if (flag_server)
        {
            Server *s = new Server();
            s->InitServ(input_str);
            Addserver(s);
        }
		else if (flag_config)
		{
			parse_config(input_str);
			flag_config = 0;
		}
		else if (flag_client)
		{
			Client *c = new Client();
			c->InitCli(input_str);
            Addclient(c);
		}
		else if (flag_request)
		{
            Request *req = new Request();
			req->Parsereq(input_str);
            req_list.push_back(req);
		}
	}
	cout<<ms;
    displaychain();
	for(vector<Request*>::iterator it = req_list.begin();it!=req_list.end();++it)
	{
        cout<<"Request is: ";
        cout<<(*it)<<endl;
        Client *c = SearchClient((*it));
        if (c == NULL)
        {
            cout<<"Do not find the client"<<endl;
            continue;
        }
        c->Setsocket();
        int sockfd = c->Getsocket();
        struct sockaddr_in srvaddr = SearchServer((*it))->Getsockaddr();
        socklen_t len = sizeof(srvaddr);
        char buf[MAXLINE];
        c->Packetize((*it), buf);
        Sendto(sockfd, buf, MAXLINE, 0, (SA*)&srvaddr, len);
        int i;
        if ((i = recvfrom(sockfd, buf, MAXLINE, 0, (SA*)&srvaddr, &len) < 0))
            cout<<"error recvfrom"<<endl;
        else
        {
            Reply *reply = new Reply();
            reply->Depacketize(buf);
            cout<<"Result is: ";
            cout<<reply<<endl;
        }
        close(sockfd);
	}
}

static Client * SearchClient(Request *req)
{
    stringstream sstream;
    sstream << req->reqID[0];
    int bankname;
    sstream >> bankname;
    std::map<int,std::vector<Client*> >::iterator it;
    it = cChain.find(bankname);
    for (vector<Client*>::iterator it1 = it->second.begin(); it1 != it->second.end(); it1++)
    {
        if (((*it1)->GetAccountno()) == (req->account_num))
            return (*it1);
    }
    return NULL;
}

static Server * SearchServer(Request *req)
{
    stringstream sstream;
    sstream << req->reqID[0];
    int bankname;
    sstream >> bankname;
    std::map<int,std::list<Server*> >::iterator it;
    it = sChain.find(bankname);
    if (req->reqtype == Query)
        return it->second.back();
    else
        return it->second.front();
}

void Addserver(Server *s)
{
    int bankname = s->GetbankName();
    std::map<int, std::list<Server*> >::iterator it;
    it=sChain.find(bankname);
    if(it!=sChain.end())
    {
        it->second.push_back(s);
    }
    else
    {
        std::list<Server*> serverchain;
        serverchain.push_back(s);
        sChain.insert(std::pair<int,std::list<Server*> >(bankname,serverchain));
    }
}

void Addclient(Client *c)
{
    int bankname = c->GetbankName();
    std::map<int, std::vector<Client*> >::iterator it;
    it=cChain.find(bankname);
    if(it!=cChain.end())
    {
        it->second.push_back(c);
    }
    else
    {
        std::vector<Client*> clientchain;
        clientchain.push_back(c);
        cChain.insert(std::pair<int,std::vector<Client*> >(bankname,clientchain));
    }
}

void displaychain()
{
    int chainnum = 1;
    for(map<int, list<Server*> >::iterator it1 = sChain.begin(); it1 != sChain.end(); ++it1, ++chainnum)
    {
        cout<<"-----------------Chain "<<chainnum<<"---------------------"<<endl;
        int count = 0;
        for(list<Server *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
        {
            if(it2 == it1->second.begin())
            {
                cout<<"Head"<<endl;
            }
            else if(count == it1->second.size() - 1)
            {
                cout<<"Tail"<<endl;
            }
            cout<<(*it2)<<endl;
        }
    }
    chainnum = 1;
    for(map<int, vector<Client*> >::iterator it1 = cChain.begin(); it1 != cChain.end(); ++it1, ++chainnum)
    {
        cout<<"-----------------Chain "<<chainnum<<"---------------------"<<endl;
        int count = 0;
        for(vector<Client *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
        {
            cout<<(*it2)<<endl;
        }
    }
}
