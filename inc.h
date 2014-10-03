#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
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

enum Outcome {Processed, InconsistentWithHistory, InsufficientFunds};
enum ReqType {Query, Deposit, Withdraw, Transfer};
enum Source  {server, client};

struct Request
{
	string reqID;
	ReqType reqtype;
	int account_num;
	int amount;
};

class InitReq
{
public:
	Source src;
	pair<string, int> src_name;
	int bankName;
};

class Reply
{
public:
	string reqID;
	Outcome outcome;
	float balance;
};

void tokenizer(string input, vector<string>& vStr)
{
	boost::split(vStr, input, boost::is_any_of(delim), boost::token_compress_on);
}
#endif
