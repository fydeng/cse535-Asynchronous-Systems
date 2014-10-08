#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <list>
#include "unp.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;
#ifndef _INC_H__
#define _INC_H__

const char *delim = ": ;,()";
const char *seperator = ",";


enum Outcome {Processed, InconsistentWithHistory, InsufficientFunds};
enum ReqType {Query, Deposit, Withdraw, Transfer};
enum Source  {server, client};

void tokenizer(string input, vector<string>& vStr)
{
    boost::split(vStr, input, boost::is_any_of(delim), boost::token_compress_on);
}

class Request
{
public:
    string req_str;
	string reqID;
	ReqType reqtype;
	int account_num;
	int amount;

    void Parsereq(string input_str)
    {
        req_str = input_str;
        char *input;
        vector<string> vStr;
        tokenizer(input_str,vStr);
        int index = 0;
        for(vector<string>::iterator it = vStr.begin(); it != vStr.end(); ++it, ++index)
        {
            input = const_cast<char *>((*it).c_str());
            if (!strlen(input))
            {
                index --;
                continue;
            }
            switch(index)
            {
                case 0:
                    if (!strncmp(input,"getBalance",10))
                    {
                        reqtype = Query;
                    }
                    else if (!strcmp(input, "deposit"))
                        reqtype = Deposit;
                    else if (!strcmp(input, "withdraw"))
                        reqtype = Withdraw;
                    else if (!strcmp(input, "Transfer"))
                        reqtype = Transfer;
                    break;
                case 1:
                    reqID = input;
                    break;
                case 2:
                    account_num = atoi(input);
                    break;
                case 3:
                    amount = atoi(input);
                    break;
                default:
                    break;
            }
        }
    }
    
    friend ostream & operator << (ostream & cout, Request *req)
    {
        if(req->reqtype == Query)
            cout<<"getBalance ";
        else if(req->reqtype == Deposit)
            cout<<"Deposit ";
        else if(req->reqtype == Withdraw)
            cout<<"Withdraw ";
        else if(req->reqtype == Transfer)
            cout<<"Transfer "<<endl;
        cout<<"req Id: "<<req->reqID<<" account_num "<<req->account_num<<" amount "<<req->amount<<endl;
        return cout;
    }
};

class Reply
{
public:
	string reqID;
	Outcome outcome;
	float balance;
};

#endif
