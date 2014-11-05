from enum import Enum

Outcome = Enum('Outcome','Processed InconsistentWithHistory InsufficientFunds')
ReqType = Enum('ReqType','Query Deposit Withdraw Transfer')
Source = Enum('Source','server client')

class Request(object):
    def __init__(self,reqID,reqtype,account_num,amount,client_id):
        self.reqID =reqID
        self.reqtype = reqtype
        self.account_num = int(account_num)
        self.amount = float(amount)
        self.client_id = client_id
    def __str__(self):
        return "RequestID is %s, %s, Account number is %d, Amount is %f" %(self.reqID, self.reqtype, self.account_num, self.amount)

class InitReq(object):
    def __init__(self,src,srcName,bankName):
        self.src =src
        self.srcName =srcName
        self.bankName =bankName

class Reply(object):
    def __init__(self,reqID,outcome,balance):
        self.reqID = reqID
        self.outcome = outcome
        self.balance = float(balance)

    def __str__(self):
        return "RequestID is %s, %s, Balance is %f" %(self.reqID, self.outcome, self.balance)