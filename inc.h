#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <list>
#include "unpthread.h"

using namespace std;

#ifndef _INC_H__
#define _INC_H__

const char *delim = ": ;,()";

enum Outcome {Processed, InconsistentWithHistory, InsufficientFunds};
enum ReqType {Query, Deposit, Withdraw, Transfer};

class Request
{
	string reqID;
	ReqType reqtype;
};

class Reply
{
	string reqID;
	Outcome outcome;
	float balance;
};

#endif
