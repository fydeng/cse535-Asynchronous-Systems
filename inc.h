#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <list>
#include <setjmp.h>
#include "unp.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;
#ifndef _INC_H__
#define _INC_H__

const char *delim = ": ; , ( )";
const char *seperator = "-----------------------------------------------------------------";


enum Outcome {Processed, InconsistentWithHistory, InsufficientFunds};
enum ReqType {Query, Deposit, Withdraw, Transfer};
enum Source  {server, client};

void tokenizer(string input, vector<string>& vStr) //tokenizer to split the attributes in configuration file
{
    boost::split(vStr, input, boost::is_any_of(delim), boost::token_compress_on);
}

class Request  //class of request, consisting variables defined in project description and some functions
{
public:
    string req_str;
	string reqID;
	ReqType reqtype;
    int bankname;
	int account_num;
	float amount;
    
    Request(){}
    Request (ReqType req_type, int bank_name, int client_no, int sequence, int account_no, float amt)
    {
        bankname = bank_name;
        amount = amt;
        reqtype = req_type;
        account_num = account_no;
        reqID = std::to_string(bankname);
        reqID.append(".");
        reqID.append(std::to_string(client_no));
        reqID.append(".");
        reqID.append(std::to_string(sequence));
        req_str = std::to_string(reqtype);
        req_str.append(",");
        req_str.append(reqID);
        req_str.append(",");
        req_str.append(std::to_string(account_num));
        req_str.append(",");
        req_str.append(std::to_string(amount));
    }
    
    Request(string input_str)
    {
		amount = 0;
        Parsereq(input_str);
    }

    void Parsereq(string input_str) //parse the request and construct the request object
    {
        req_str = input_str;
        char *input;
        vector<string> vStr;
        tokenizer(input_str,vStr);
        int index = 0;
        stringstream sstream;
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
                    if (((*input) >= '0') && ((*input) < '4'))
                        reqtype = (enum ReqType)(atoi(input));
                    else if (!strncmp(input,"getBalance",10))
                        reqtype = Query;
                    else if (!strcmp(input, "deposit"))
                        reqtype = Deposit;
                    else if (!strcmp(input, "withdraw"))
                        reqtype = Withdraw;
                    else if (!strcmp(input, "Transfer"))
                        reqtype = Transfer;
                    break;
                case 1:
                    reqID = input;
                    sstream << reqID[0];
                    sstream >> bankname;
                    break;
                case 2:
                    account_num = atoi(input);
                    break;
                case 3:
                    amount = atof(input);
                    break;
                default:
                    break;
            }
        }
    }
    
    bool operator == (Request *req) //some overiding operators
    {
        if ((!(reqID.compare(req->reqID))) &&  (reqtype == req->reqtype))
            return true;
        else
            return false;
    }
    
    friend ostream & operator << (ostream & cout, Request *req)
    {
        printf("Request type: ");
        if(req->reqtype == Query)
            printf("getBalance ");
        else if(req->reqtype == Deposit)
            printf("Deposit ");
        else if(req->reqtype == Withdraw)
            printf("Withdraw ");
        else if(req->reqtype == Transfer)
            printf("Transfer ");
        //cout<<"Req Id: "<<req->reqID<<" Account number: "<<req->account_num<<" Amount: "<<req->amount<<endl;
        printf("Req ID: %s Account number: %d Amount: %f\n", req->reqID.c_str(),req->account_num,req->amount);
        return cout;
        /*cout<<"Request type: ";
        if(req->reqtype == Query)
            cout<<"getBalance ";
        else if(req->reqtype == Deposit)
            cout<<"Deposit ";
        else if(req->reqtype == Withdraw)
            cout<<"Withdraw ";
        else if(req->reqtype == Transfer)
            cout<<"Transfer "<<endl;
        cout<<"Req Id: "<<req->reqID<<" Account number: "<<req->account_num<<" Amount: "<<req->amount<<endl;
        return cout;*/

    }
};

class Reply //class of reply, consisting variables defined in project description and some functions
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
    void Packetize(char *buf) //packetize the reply message
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
    void Depacketize(char *buf) //depacketize the reply message
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
    
    friend ostream & operator << (ostream & cout, Reply *reply) //operator overide
    {
        printf("Result: ");
        if(reply->outcome == Processed)
            printf("Processed ");
        else if(reply->outcome == InsufficientFunds)
            printf("InsufficientFunds ");
        else if(reply->outcome == InconsistentWithHistory)
            printf("InconsistentWithHistory ");
        printf("Req ID: %s Balance: %f\n", reply->reqID.c_str(), reply->balance);
        return cout;
        /*cout<<"Result: ";
        if(reply->outcome == Processed)
            cout<<"Processed ";
        else if(reply->outcome == InsufficientFunds)
            cout<<"InsufficientFunds ";
        else if(reply->outcome == InconsistentWithHistory)
            cout<<"InconsistentWithHistory ";
        cout<<"Req Id: "<<reply->reqID<<" Balance: "<<reply->balance<<endl;
        return cout;*/

    }
};

class ACK //class of ACK
{
public:
	string reqID;

	ACK(Request *req)
	{
		reqID = req->reqID;
	}

	ACK(char *buf)
	{
		reqID = buf;
	}

	void Packetize(char *buf)
	{
		strcpy(buf, reqID.c_str());
	}

    friend ostream & operator << (ostream & cout, ACK *ack)
    {
        printf("ACK for reqID: %s has been received\n", ack->reqID.c_str());
		return cout;
    }

};
#endif
