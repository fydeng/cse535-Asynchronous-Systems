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
    
    Request(){}
    Request(string input_str)
    {
        Parsereq(input_str);
    }

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
    
    bool operator == (Request *req) const
    {
        if ((!(reqID.compare(req->reqID))) &&  (reqtype == req->reqtype))
            return true;
        else
            return false;
    }
    
    friend ostream & operator << (ostream & cout, Request *req)
    {
        cout<<"Request type: ";
        if(req->reqtype == Query)
            cout<<"getBalance ";
        else if(req->reqtype == Deposit)
            cout<<"Deposit ";
        else if(req->reqtype == Withdraw)
            cout<<"Withdraw ";
        else if(req->reqtype == Transfer)
            cout<<"Transfer "<<endl;
        cout<<"Req Id: "<<req->reqID<<" Account number: "<<req->account_num<<" Amount: "<<req->amount<<endl;
        return cout;
    }
};

class Reply
{
public:
	string reqID;
	Outcome outcome;
	float balance;
    Reply(){}
    Reply(Request *req)
    {
        reqID = req->reqID;
    }
    void Packetize(char *buf)
    {
        string str;
        string str1;
        stringstream sstream;
        str = reqID;
        sstream << ",";
        sstream << outcome;
        sstream << ",";
        sstream << balance;
        sstream >> str1;
        str.append(str1);
        strcpy(buf, str.c_str());
    }
    void Depacketize(char *buf)
    {
        string input_str = buf;
        char *input;
        vector<string> vStr;
        tokenizer(input_str,vStr);
        int index = 0;
        for(vector<string>::iterator it = vStr.begin(); it != vStr.end(); ++it, ++index)
        {
            input = const_cast<char *>((*it).c_str());
            switch(index)
            {
                case 0:
                    reqID = input;
                    break;
                case 1:
                    outcome = (enum Outcome)(atoi(input));
                    break;
                case 2:
                    balance = atof(input);
                    break;
                default:
                    break;
            }
        }
    }
    
    friend ostream & operator << (ostream & cout, Reply *reply)
    {
        cout<<"Result: ";
        if(reply->outcome == Processed)
            cout<<"Processed ";
        else if(reply->outcome == InsufficientFunds)
            cout<<"InsufficientFunds ";
        else if(reply->outcome == InconsistentWithHistory)
            cout<<"InconsistentWithHistory ";
        cout<<"Req Id: "<<reply->reqID<<" Balance: "<<reply->balance<<endl;
        cout<<"------------------------------------";
        return cout;
    }
};

#endif
