#include "server.h"
class Master
{
private:
	std::pair<string,int> mName;
	std::map<int, std::list<Server*> > sChain;
	//std::map<int, std::list<Client*> > clients;

public:
	void InitMS(ifstream &fin);
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
	map<int, std::list<Server*> > & Getschain()
	{
		return sChain;
	}
	friend ostream & operator << (ostream & cout, Master &ms)
	{
		int chainnum = 1;
		for(map<int, list<Server*> >::iterator it1 = ms.Getschain().begin(); it1 != ms.Getschain().end(); ++it1, ++chainnum)
		{
			cout<<"-----------------Chain "<<chainnum<<"---------------------"<<endl;
			int count = 0;
			for(list<Server *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2,++count)
			{
				if(it2 == it1->second.begin())
				{
					cout<<"front"<<endl;
				}
				else if(count == it1->second.size() - 1)
				{
					cout<<"back"<<endl;
				}
				cout<<(*it2)<<endl;
			}
		}
		return cout;
	}
};
