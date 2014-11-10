extern "C"
{
#include "unpthread.h"
}
#include "client.h"
#include "master.h"
#include "server.h"

std::map<int, std::vector<Client*> >cChain; //client map
std::map<int, std::list<Server*> > sChain; //server map

void Addserver(Server *s);
void Addclient(Client *c);
void Addrequest(Request *req);
static Client * SearchClient(Request *);
static Server * SearchServer(Request *);
static void *SendReq(void *);
int readable_timeo(int, int);
void displaychain(bool, bool);
void GenerateRanReq();
bool proc_push_notification(char *buf);
void remove_server(Push_Notification *n);
void chain_extension(Push_Notification *n);
void cal_next(int);

struct ARGS //struct of args, used to pass arguments the new thread
{
    Client *client;
    Master *master;
    
    ARGS(Client *cli, Master *ms)
    {
        client = cli;
        master = ms;
    }
};

int main(int argc, char **argv)
{
	ifstream fin;
	fin.open(argv[1]);
	string input_str;
	char *input;
	Master *ms = new Master();
	int flag_master = 0, flag_client = 0, flag_config = 0, flag_request = 0, flag_server = 0;
    int client_num = 0;
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
            if (!s->Getdelay())
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
            client_num++;
		}
		else if (flag_request)
		{
            if (random_req)
            {
                parse_randomized_req(input_str); //parse randomized request
                break;
            }
            else
            {
                Request *req = new Request(input_str);
                Addrequest(req);
            }
		}
	}
//	cout<<ms;
    displaychain(true, true);
    if (random_req)
        GenerateRanReq();
    cout<<"Number of client is "<<client_num<<endl;
	cout<<endl<<"-------------Now start sending requests-------------"<<endl;
    pthread_t tid[client_num];
    int index = 0;
    for(std::map<int, vector<Client*> >::iterator it = cChain.begin(); it!=cChain.end(); ++it)
    {
        for(std::vector<Client*>::iterator it1 = it->second.begin(); it1!=it->second.end(); ++it1, ++index)
        {
            sleep(1);
            ARGS args((*it1),ms);
            Pthread_create(&tid[index], NULL, &SendReq, (void *)&args);
            Pthread_join(tid[index], NULL);
        }
    }
    /*for (index = 0; index < client_num; index++)
    {
        Pthread_join(tid[index], NULL);
    }*/
    cout<<"All clients' requests have been sent, clients exit"<<endl;
    return 1;
}

static void *SendReq(void *arg) //each client launches this send request function
{
    Client *c = ((struct ARGS *)arg)->client;
    Master *ms = ((struct ARGS *)arg)->master;
    cout<<c<<endl;
    c->Setsocket();
    int sockfd = c->Getsocket();
    socklen_t len = sizeof(struct sockaddr_in);
    int count_retrans;
    for (list<Request*>::iterator it = c->GetReqList().begin(); it != c->GetReqList().end(); it++)
    {
        count_retrans = 0;
        sleep(1);
    loop:        cout<<(*it)<<endl;
        char buf[MAXLINE];
        Server *s = SearchServer((*it));
        if (s == NULL)
        {
            printf("There is no server in the chain, client exit\n");
            break;
        }
        struct sockaddr_in srvaddr = s->Getsockaddr();
        c->Packetize((*it), buf);
        Sendto(sockfd, buf, MAXLINE, 0, (SA*)&srvaddr, len);
    receiving:        if (readable_timeo(sockfd, retrans_inteval)==0)
        {
            if (count_retrans == retrans_time)
            {
                cout<<"Retransmit time equals the limit, stop retransmit"<<endl;
                continue;
            }
            else
            {
                cout<<"Message timeout, start retransmit"<<endl;
                count_retrans++;
                goto loop;
            }
        }
        else
        {
            if ((recvfrom(sockfd, buf, MAXLINE, 0, (SA*)&srvaddr, &len) < 0))
                cout<<"error recvfrom"<<endl;
            else
            {
                int port = ntohs(srvaddr.sin_port);
                if (port == ms->GetmsName().second)
                {
                    printf("%s\npush notification received from master\n", seperator);
                    bool result;
                    result = proc_push_notification(buf);
                    displaychain(true, false);
                    if (result) //there is server removed
                    {
                        sleep(2);
                        count_retrans = 0;
                        goto loop;
                    }
                    else
                    {
                        sleep(10);
                        goto receiving;
                    }
                }
                else
                {
                    Reply *reply = new Reply();
                    reply->Depacketize(buf);
                    cout<<reply<<endl;
                }
        
            }
        }
    }
    close(sockfd);
    return NULL;
}

static Client * SearchClient(Request *req) //search client according to specific request
{
    std::map<int,std::vector<Client*> >::iterator it;
    it = cChain.find(req->bankname);
    for (vector<Client*>::iterator it1 = it->second.begin(); it1 != it->second.end(); it1++)
    {
        if (((*it1)->GetAccountno()) == (req->account_num))
            return (*it1);
    }
    return NULL;
}

static Server * SearchServer(Request *req) //search server according to specific request
{
    if (sChain.empty())
        return NULL;
    std::map<int,std::list<Server*> >::iterator it;
    it = sChain.find(req->bankname);
    if (req->reqtype == Query)
        return it->second.back();
    else
        return it->second.front();
}

void Addserver(Server *s) //add server to server map
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

void Addclient(Client *c) //add client to client map
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

void Addrequest(Request *req) //add request to client's request list
{
    std::map<int, std::vector<Client*> >::iterator it;
    it=cChain.find(req->bankname);
    if(it!=cChain.end())
    {
        for(vector<Client *>::iterator it1 = it->second.begin(); it1 != it->second.end(); ++it1)
        {
            if (((*it1)->GetAccountno()) == (req->account_num))
            {
                (*it1)->Addrequest(req);
                return;
            }
        }
    }
    else
        return;
}

void GenerateRanReq() //generate request for clients
{
    int index;
    for(map<int, vector<Client*> >::iterator it1 = cChain.begin(); it1 != cChain.end(); ++it1)
    {
        index = 1;
        for(vector<Client *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2, ++index)
        {
            (*it2)->GenerateRandomReq(index);
        }
    }
}

bool proc_push_notification(char *buf)
{
    Push_Notification *noti = new Push_Notification(buf);
    cout<<noti<<endl;
    if (noti->noti_type == Fail)
    {
        remove_server(noti);
        return true;
    }
    else
    {
        chain_extension(noti);
        return false;
    }
}

void chain_extension(Push_Notification *n)
{
    pair<int,int> srv = make_pair(n->bankname, n->port_num);
    Server *s = new Server(srv);
    Addserver(s);
    cal_next(n->bankname);
}

void remove_server(Push_Notification *n)
{
    int bankname = n->bankname;
    std::map<int, std::list<Server*> >::iterator it;
    it=sChain.find(bankname);
    for(list<Server *>::iterator it1 = (it->second).begin(); it1 != (it->second).end(); ++it1)
    {
        if (((*it1)->GetserverName().second == n->port_num))
        {
            it1 = it->second.erase(it1);
            //return;
            break;
        }
    }
    cal_next(bankname);
}

void cal_next(int bankname)
{
    map<int, list<Server*> >::iterator it1 = sChain.find(bankname);
    int count = 0;
    for(list<Server *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
    {
        list<Server *>::iterator cur = it2;
        if(count == it1->second.size() - 1)
        {
            (*cur)->Setnext(NULL);
            continue;
        }
        else
        {
            (*cur)->Setnext(*(++it2));
            
        }
        it2 = cur;
    }
}

void displaychain(bool display_server, bool display_client) //display server map and client map
{
    int chainnum = 1;
    if (display_server)
    {
        cout<<"----------------Display Server Chain---------------------"<<endl;
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
                if(count == it1->second.size() - 1)
                {
                    cout<<"Tail"<<endl;
                }
                cout<<(*it2)<<endl;
            }
        }
    }
    
    if (display_client)
    {
        chainnum = 1;
        cout<<"----------------Display Client Chain---------------------"<<endl;
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
}

int readable_timeo(int fd, int sec)
{
    fd_set rset;
    struct timeval tv;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    tv.tv_sec = sec;
    tv.tv_usec = 0;
    
    return(select(fd + 1, &rset, NULL, NULL, &tv));
}
