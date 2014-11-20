
import da
from enum import Enum
Outcome = Enum('Outcome', 'Processed InconsistentWithHistory InsufficientFunds')
ReqType = Enum('ReqType', 'Query Deposit Withdraw Transfer')
Source = Enum('Source', 'server client')

class Request(object):

    def __init__(self, reqID, reqtype, account_num, amount, client_id):
        self.reqID = reqID
        self.reqtype = reqtype
        self.account_num = int(account_num)
        self.amount = float(amount)
        self.client_id = client_id

    def __str__(self):
        return ('RequestID is %s, %s, Account number is %d, Amount is %f' % (self.reqID, self.reqtype, self.account_num, self.amount))

class TransferRequest(object):

    def __init__(self, reqID, reqtype, from_bank, from_account, to_bank, to_account, amount, client_id):
        self.reqID = reqID
        self.reqtype = reqtype
        self.from_bank = int(from_bank)
        self.from_account = int(from_account)
        self.to_bank = int(to_bank)
        self.to_account = int(to_account)
        self.amount = float(amount)
        self.client_id = client_id

    def __str__(self):
        return ('TransferRequestID is %s %s, From Bank: %d Account: %d, To Bank: %d Account: %d, Amount is %f' % (self.reqID, self.reqtype, self.from_bank, self.from_account, self.to_bank, self.to_account, self.amount))

class InitReq(object):

    def __init__(self, src, srcName, bankName):
        self.src = src
        self.srcName = srcName
        self.bankName = bankName

class Reply(object):

    def __init__(self, reqID, outcome, balance):
        self.reqID = reqID
        self.outcome = outcome
        self.balance = float(balance)

    def __str__(self):
        return ('RequestID is %s, %s, Balance is %f' % (self.reqID, self.outcome, self.balance))

class Ping(object):

    def __init__(self, serverIP, bankName):
        self.serverIP = serverIP
        self.bankName = bankName

    def __str__(self):
        return ('Ping from %s' % self.serverIP)

class Ack(object):

    def __init__(self, reqID, reqtype, account_num, amount):
        self.reqID = reqID
        self.reqtype = reqtype
        self.account_num = int(account_num)
        self.amount = float(amount)

    def __str__(self):
        return ('Ack for request %s' % self.reqID)

class Transfer_Ack(object):

    def __init__(self, reqID, reqtype, from_bank, from_account, to_bank, to_account, amount, client_id):
        self.reqID = reqID
        self.reqtype = reqtype
        self.from_bank = int(from_bank)
        self.from_account = int(from_account)
        self.to_bank = int(to_bank)
        self.to_account = int(to_account)
        self.client_id = client_id
        self.amount = float(amount)

    def __str__(self):
        return ('Transfer Ack for %s' % self.reqID)
