
import da
PatternExpr_0 = da.pat.TuplePattern([da.pat.ConstantPattern('PING'), da.pat.FreePattern('ping')])
PatternExpr_1 = da.pat.FreePattern('src_id')
PatternExpr_2 = da.pat.TuplePattern([da.pat.ConstantPattern('REQ'), da.pat.FreePattern('req')])
PatternExpr_3 = da.pat.FreePattern('src_id')
PatternExpr_4 = da.pat.TuplePattern([da.pat.ConstantPattern('SYNC'), da.pat.FreePattern('req')])
PatternExpr_5 = da.pat.FreePattern('src_id')
PatternExpr_6 = da.pat.TuplePattern([da.pat.ConstantPattern('SYNC_PROC'), da.pat.TuplePattern([da.pat.FreePattern('reqID'), da.pat.FreePattern('req')])])
PatternExpr_7 = da.pat.FreePattern('src_id')
PatternExpr_8 = da.pat.TuplePattern([da.pat.ConstantPattern('ACK'), da.pat.FreePattern('ack')])
PatternExpr_9 = da.pat.FreePattern('next')
PatternExpr_10 = da.pat.TuplePattern([da.pat.ConstantPattern('srvFail'), da.pat.TuplePattern([da.pat.FreePattern('prev'), da.pat.FreePattern('next')])])
PatternExpr_11 = da.pat.FreePattern('master')
PatternExpr_12 = da.pat.TuplePattern([da.pat.ConstantPattern('extendChain'), da.pat.FreePattern('newSrv')])
PatternExpr_13 = da.pat.FreePattern('master')
PatternExpr_14 = da.pat.TuplePattern([da.pat.ConstantPattern('new_server_join'), da.pat.FreePattern('newprev')])
PatternExpr_15 = da.pat.FreePattern('master')
PatternExpr_16 = da.pat.TuplePattern([da.pat.ConstantPattern('REPLY'), da.pat.FreePattern('reply')])
PatternExpr_17 = da.pat.FreePattern('src_id')
PatternExpr_18 = da.pat.TuplePattern([da.pat.ConstantPattern('newHead'), da.pat.FreePattern('newSrv')])
PatternExpr_19 = da.pat.FreePattern('master')
PatternExpr_20 = da.pat.TuplePattern([da.pat.ConstantPattern('newTail'), da.pat.FreePattern('newSrv')])
PatternExpr_21 = da.pat.FreePattern('master')
PatternExpr_22 = da.pat.TuplePattern([da.pat.ConstantPattern('srvFail'), da.pat.FreePattern('newSrv')])
PatternExpr_23 = da.pat.FreePattern('master')
PatternExpr_24 = da.pat.TuplePattern([da.pat.ConstantPattern('REPLY'), da.pat.FreePattern('reply')])
PatternExpr_25 = da.pat.FreePattern('dst')
PatternExpr_26 = da.pat.FreePattern('rclk')
PatternExpr_28 = da.pat.TuplePattern([da.pat.ConstantPattern('REPLY'), da.pat.FreePattern('reply')])
PatternExpr_29 = da.pat.FreePattern('dst')
PatternExpr_30 = da.pat.FreePattern('rclk')
import re
import time
import sys
import collections
from init import *
import random

def parse_config(input_str):
    input_str = input_str.replace(' ', '')
    strs = input_str.split(',')
    if (not (int(strs[0]) == 0)):
        retrans = True
        print('Retransmission enabled')
    else:
        reTrans = False
    retrans_interval = int(strs[1])
    print('Timeout is:', retrans_interval)
    retrans_time = int(strs[2])
    print('nRetrans is:', retrans_time)
    if (not (int(strs[3]) == 0)):
        ifRandom = True
        print('Random request enabled')
    else:
        ifRandom = False
    return [retrans, retrans_interval, retrans_time, ifRandom]

def parse_server(input_str):
    input_str = input_str.strip()
    input_str = input_str.replace(' ', '')
    str = input_str.split(',')
    bankName = int(str[0])
    serverIP = str[1]
    startup_delay = int(str[2])
    life_time = str[3]
    return [bankName, serverIP, startup_delay, life_time]

def parse_client(input_str):
    input_str = input_str.replace(' ', '')
    str = input_str.split(',')
    bankName = str[0]
    account_no = str[1]
    clientIP = str[2]
    return [bankName, account_no, clientIP]

def parse_req(input_str):
    input_str = input_str.replace(' ', '')
    input_str = re.sub('[()]', '', input_str)
    strs = input_str.split(',')
    if (strs[0] == 'getBalance'):
        reqtype = ReqType.Query
    elif (strs[0] == 'deposit'):
        reqtype = ReqType.Deposit
    elif (strs[0] == 'withdraw'):
        reqtype = ReqType.Withdraw
    else:
        reqtype = ReqType.Transfer
    reqID = strs[1]
    account_num = strs[2]
    if (reqtype == ReqType.Query):
        amount = 0
    else:
        amount = strs[3]
    return [reqID, reqtype, account_num, amount]

def parse_randomReq(input_str):
    input_str = input_str.strip()
    input_str = input_str.replace(' ', '')
    input_str = re.sub('[()]', '', input_str)
    strs = input_str.split(',')
    seed_num = int(strs[0])
    num_req = int(strs[1])
    prob_query = float(strs[2])
    prob_deposit = float(strs[3])
    prob_withdraw = float(strs[4])
    prob_transfer = float(strs[5])
    return [seed_num, num_req, prob_query, prob_deposit, prob_withdraw, prob_transfer]

class Master(da.DistProcess):

    def __init__(self, parent, initq, channel, props):
        super().__init__(parent, initq, channel, props)
        self._events.extend([da.pat.EventPattern(da.pat.ReceivedEvent, '_MasterReceivedEvent_0', PatternExpr_0, sources=[PatternExpr_1], destinations=None, timestamps=None, record_history=None, handlers=[self._Master_handler_0])])

    def setup(self, srvDic, cliDic, filename, waitList):
        self.waitList = waitList
        self.cliDic = cliDic
        self.filename = filename
        self.srvDic = srvDic
        self.timesheet = {}
        self.srvDict = self.srvDic
        self.cliDict = self.cliDic
        self.filename = self.filename
        self.waitList = self.waitList

    def main(self):
        self.output((('Master  ' + str(self.id)) + '  has started.'))
        print('SERVER DICTIONARY STORED IN MASTER', self.srvDict)
        print('CLIENT DICTIONARY STORED IN MASTER', self.cliDict)
        print('SERVER WAITLIST STORED IN MASTER', self.waitList)
        while True:
            _st_label_171 = 0
            self._timer_start()
            while (_st_label_171 == 0):
                _st_label_171 += 1
                if False:
                    pass
                    _st_label_171 += 1
                elif self._timer_expired:
                    self.checkSrv(self.timesheet)
                    _st_label_171 += 1
                else:
                    super()._label('_st_label_171', block=True, timeout=5)
                    _st_label_171 -= 1
            else:
                if (_st_label_171 != 2):
                    continue
            if (_st_label_171 != 2):
                break

    def setup_logfile(self, filename):
        rootlog = logging.getLogger('')
        filelvl = logging.INFO
        fh = logging.FileHandler(filename)
        formatter = logging.Formatter('[%(asctime)s]%(name)s: %(message)s')
        fh.setFormatter(formatter)
        fh.setLevel(filelvl)
        rootlog._filelvl = filelvl
        rootlog.addHandler(fh)

    def flush_srvDict(self, bankname):
        for (i, item) in enumerate(self.srvDict[bankname]):
            if (len(self.srvDict[bankname]) == 1):
                item[1] = None
                item[2] = None
                return
            if (i == 0):
                item[1] = None
                item[2] = self.srvDict[bankname][(i + 1)][0]
                continue
            elif (i == (len(self.srvDict[bankname]) - 1)):
                item[2] = None
                item[1] = self.srvDict[bankname][(i - 1)][0]
            else:
                item[1] = self.srvDict[bankname][(i - 1)][0]
                item[2] = self.srvDict[bankname][(i + 1)][0]

    def infoCli(self, bankName, type, newSrv):
        for cli in self.cliDict[str(bankName)]:
            if (type == 'head'):
                print('SENDING NEW HEAD TO CLIENT:', cli)
                self._send(('newHead', newSrv), cli)
            elif (type == 'tail'):
                self._send(('newTail', newSrv), cli)
            elif (type == 'srvFail'):
                self._send(('srvFail', newSrv), cli)

    def updateSrvInfo(self, srv):
        port = srv[0]
        bankName = srv[1]
        for item in self.srvDict[bankName]:
            if (port == item[3]):
                prev = item[1]
                next = item[2]
                self.srvDict[bankName].remove(item)
                self.flush_srvDict(bankName)
                if (prev == None):
                    self.infoCli(bankName, 'head', next)
                if (next == None):
                    self.infoCli(bankName, 'tail', prev)
                elif ((not (prev == None)) and (not (next == None))):
                    self.infoCli(bankName, 'srvFail', None)
        for item in self.srvDict[bankName]:
            if ((item[0] == prev) or (item[0] == next)):
                self._send(('srvFail', (item[1], item[2])), item[0])
            else:
                pass

    def checkSrv(self, timesheet):
        time_chec = self.logical_clock()
        failedSrv = []
        for key in timesheet.keys():
            if ((time_chec - timesheet.get(key)) > 5):
                print('Server ', key[0], ' has failed!')
                failedSrv.append(key)
        for srv in failedSrv:
            del timesheet[srv]
            self.updateSrvInfo(srv)

    def _Master_handler_0(self, ping, src_id):
        time_recv = self.logical_clock()
        flag = False
        print('Received PING from: ', ping.serverIP)
        bankname = ping.bankName
        new_tail = None
        for item in self.srvDict[bankname]:
            if (ping.serverIP == item[3]):
                flag = True
                self.timesheet.update({(ping.serverIP, ping.bankName): time_recv})
            else:
                pass
        if (flag == False):
            for item in self.waitList:
                if (ping.serverIP == item[4]):
                    new_tail = item
                    print('Extending Chain, adding new server ', item[4], ' to srvDic')
                    self.srvDict[item[0]].append([item[1], item[2], item[3], item[4]])
                    self.waitList.remove(item)
                    self.flush_srvDict(bankname)
                    print('AFTER CHAIN EXTENSION', self.srvDict)
        if (not (new_tail == None)):
            self.infoCli(bankname, 'tail', new_tail[1])
            for item in self.srvDict[bankname]:
                if (item[2] == new_tail[1]):
                    self._send(('extendChain', item[2]), item[0])
                if (item[0] == new_tail[1]):
                    self._send(('new_server_join', item[1]), item[0])
    _Master_handler_0._labels = None
    _Master_handler_0._notlabels = None

class Server(da.DistProcess):

    def __init__(self, parent, initq, channel, props):
        super().__init__(parent, initq, channel, props)
        self._events.extend([da.pat.EventPattern(da.pat.ReceivedEvent, '_ServerReceivedEvent_0', PatternExpr_2, sources=[PatternExpr_3], destinations=None, timestamps=None, record_history=None, handlers=[self._Server_handler_1]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ServerReceivedEvent_1', PatternExpr_4, sources=[PatternExpr_5], destinations=None, timestamps=None, record_history=None, handlers=[self._Server_handler_2]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ServerReceivedEvent_2', PatternExpr_6, sources=[PatternExpr_7], destinations=None, timestamps=None, record_history=None, handlers=[self._Server_handler_3]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ServerReceivedEvent_3', PatternExpr_8, sources=[PatternExpr_9], destinations=None, timestamps=None, record_history=None, handlers=[self._Server_handler_4]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ServerReceivedEvent_4', PatternExpr_10, sources=[PatternExpr_11], destinations=None, timestamps=None, record_history=None, handlers=[self._Server_handler_5]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ServerReceivedEvent_5', PatternExpr_12, sources=[PatternExpr_13], destinations=None, timestamps=None, record_history=None, handlers=[self._Server_handler_6]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ServerReceivedEvent_6', PatternExpr_14, sources=[PatternExpr_15], destinations=None, timestamps=None, record_history=None, handlers=[self._Server_handler_7])])

    def setup(self, bankName, serverIP, startup_delay, life_time, prev, next, master):
        self.bankName = bankName
        self.prev = prev
        self.serverIP = serverIP
        self.master = master
        self.startup_delay = startup_delay
        self.life_time = life_time
        self.next = next
        self.bankName = self.bankName
        self.serverIP = self.serverIP
        self.startup_delay = self.startup_delay
        self.life_time = int(self.life_time)
        self.prev = self.prev
        self.next = self.next
        self.master = self.master
        self.sentTrans = []
        self.procTrans = {}
        self.AccountInfo = {}

    def main(self):
        time.sleep(self.startup_delay)
        self.output(((((((((('Server: Bank Name is: ' + str(self.bankName)) + '  Server IP is: ') + str(self.serverIP)) + '  Life time is: ') + str(self.life_time)) + '  Previous server is: ') + str(self.prev)) + '  Next server is: ') + str(self.next)))
        ping = Ping(self.serverIP, self.bankName)
        while True:
            _st_label_295 = 0
            self._timer_start()
            while (_st_label_295 == 0):
                _st_label_295 += 1
                if False:
                    pass
                    _st_label_295 += 1
                elif self._timer_expired:
                    self._send(('PING', ping), self.master)
                    _st_label_295 += 1
                else:
                    super()._label('_st_label_295', block=True, timeout=1)
                    _st_label_295 -= 1
            else:
                if (_st_label_295 != 2):
                    continue
            if (_st_label_295 != 2):
                break

    def proc_balance(self, req):
        outcome = Outcome.Processed
        if (self.AccountInfo.get(req.account_num) == None):
            cur_balance = 0
        else:
            cur_balance = self.AccountInfo[req.account_num]
        if (not (self.procTrans.get(req.reqID) == None)):
            if (not (self.procTrans[req.reqID].reqtype == req.reqtype)):
                outcome = Outcome.InconsistentWithHistory
            else:
                pass
            new_balance = cur_balance
        elif (req.reqtype == ReqType.Query):
            new_balance = cur_balance
        elif (req.reqtype == ReqType.Deposit):
            new_balance = (cur_balance + req.amount)
        elif (req.reqtype == ReqType.Withdraw):
            if ((cur_balance - req.amount) < 0):
                new_balance = cur_balance
                outcome = Outcome.InsufficientFunds
            else:
                new_balance = (cur_balance - req.amount)
        if (not (req.reqtype == ReqType.Query)):
            self.sentTrans.append(req)
            self.output(('Request %s has been added to sent transaction' % req.reqID))
        self.AccountInfo.update({req.account_num: new_balance})
        return [new_balance, outcome]

    def update_procTrans(self, reqID):
        count_clear = 0
        len_sentTrans = len(self.sentTrans)
        for i in range(len_sentTrans):
            if (self.sentTrans[i].reqID == reqID):
                req = self.sentTrans[i]
                self.sentTrans.pop(i)
                if (not (count_clear == 0)):
                    self.output('Using ACK to clear old req ', reqID)
                count_clear += 1
                if (req.reqtype == ReqType.Query):
                    break
                proc_req = self.procTrans.get(req.reqID)
                if (proc_req == None):
                    self.procTrans.update({req.reqID: req})
                    print('Requst ', req.reqID, 'has been added to server ', self.serverIP, 'processed transaction')
                elif (req.reqtype == proc_req.reqtype):
                    self.output('Duplicate request, donot add to processed transaction')
                else:
                    self.output('Inconsistent With Histroy request, donot add to processed transaction')
                break

    def _Server_handler_1(self, src_id, req):
        self.output((('Request ' + str(req.reqID)) + ' from client received! '))
        if (self.life_time > 0):
            self.life_time = (self.life_time - 1)
            self.output(('the current life_time is: ' + str(self.life_time)))
        elif (self.life_time == 0):
            self.output((('Server: ' + str(self.serverIP)) + ' has expired!'))
            sys.exit()
        result = self.proc_balance(req)
        new_balance = result[0]
        outcome = result[1]
        reply = Reply(req.reqID, outcome, new_balance)
        ack = Ack(req.reqID)
        if (self.prev == None):
            self._send(('REQ', req), self.next)
        elif (self.next == None):
            self.update_procTrans(req.reqID)
            self._send(('REPLY', reply), req.client_id)
            if (not (req.reqtype == ReqType.Query)):
                self._send(('ACK', ack), self.prev)
                self.output((str(ack) + ' has been sent!'))
        else:
            self._send(('REQ', req), self.next)
        self.output(('Reply for Request %s has been sent' % reply.reqID))
    _Server_handler_1._labels = None
    _Server_handler_1._notlabels = None

    def _Server_handler_2(self, req, src_id):
        self.output('Synchronization of sentTrans from previous server received! ReqID is ', req.reqID)
        if (not (req.reqtype == ReqType.Query)):
            self.sentTrans.append(req)
        self.output(('Request %s has been added to sent transaction' % req.reqID))
        if (not (self.next == None)):
            self._send(('SYNC', req), self.next)
    _Server_handler_2._labels = None
    _Server_handler_2._notlabels = None

    def _Server_handler_3(self, src_id, req, reqID):
        print('Synchronization of procTrans from the old tail ', reqID)
        if (not (req.reqtype == ReqType.Query)):
            self.sentTrans.append(req)
            self.update_procTrans(reqID)
        if (not (self.next == None)):
            self._send(('SYNC_PROC', (reqID, req)), self.next)
    _Server_handler_3._labels = None
    _Server_handler_3._notlabels = None

    def _Server_handler_4(self, ack, next):
        self.output((str(ack) + ' has been received!'))
        self.update_procTrans(ack.reqID)
    _Server_handler_4._labels = None
    _Server_handler_4._notlabels = None

    def _Server_handler_5(self, next, prev, master):
        print("I'm the prev or next of the failed server, changing my prev and next...")
        self.prev = prev
        self.next = next
        print('Setting my prev to: ', self.prev, ' and my next to: ', self.next)
        for i in range(len(self.sentTrans)):
            req = self.sentTrans[i]
            print(('Starting synchronize %s sentTrans to next server' % req.reqID))
            self._send(('SYNC', req), self.next)
    _Server_handler_5._labels = None
    _Server_handler_5._notlabels = None

    def _Server_handler_6(self, newSrv, master):
        print(self.id, ' Received extendChain from master,setting my next to ', newSrv)
        self.next = newSrv
        print('HHHHHHHHHHHHHHHH', self.procTrans)
        for (reqID, req) in self.procTrans.items():
            print('Starting synchronize procTrans to the new tail ', reqID)
            self._send(('SYNC_PROC', (reqID, req)), self.next)
    _Server_handler_6._labels = None
    _Server_handler_6._notlabels = None

    def _Server_handler_7(self, master, newprev):
        print(self.id, 'Received new server join notification from master,setting my prev to ', newprev)
        self.prev = newprev
        self.next = None
    _Server_handler_7._labels = None
    _Server_handler_7._notlabels = None

class Client(da.DistProcess):

    def __init__(self, parent, initq, channel, props):
        super().__init__(parent, initq, channel, props)
        self._ClientReceivedEvent_4 = []
        self._ClientReceivedEvent_5 = []
        self._events.extend([da.pat.EventPattern(da.pat.ReceivedEvent, '_ClientReceivedEvent_0', PatternExpr_16, sources=[PatternExpr_17], destinations=None, timestamps=None, record_history=None, handlers=[self._Client_handler_8]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ClientReceivedEvent_1', PatternExpr_18, sources=[PatternExpr_19], destinations=None, timestamps=None, record_history=None, handlers=[self._Client_handler_9]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ClientReceivedEvent_2', PatternExpr_20, sources=[PatternExpr_21], destinations=None, timestamps=None, record_history=None, handlers=[self._Client_handler_10]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ClientReceivedEvent_3', PatternExpr_22, sources=[PatternExpr_23], destinations=None, timestamps=None, record_history=None, handlers=[self._Client_handler_11]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ClientReceivedEvent_4', PatternExpr_24, sources=[PatternExpr_25], destinations=None, timestamps=[PatternExpr_26], record_history=True, handlers=[]), da.pat.EventPattern(da.pat.ReceivedEvent, '_ClientReceivedEvent_5', PatternExpr_28, sources=[PatternExpr_29], destinations=None, timestamps=[PatternExpr_30], record_history=True, handlers=[])])

    def setup(self, bankName, account_no, clientIP, input_req, ifRetrans, timeout, nRetrans, ifRandom, master, head_srvs, tail_srvs):
        self.bankName = bankName
        self.timeout = timeout
        self.account_no = account_no
        self.ifRandom = ifRandom
        self.clientIP = clientIP
        self.head_srvs = head_srvs
        self.tail_srvs = tail_srvs
        self.master = master
        self.input_req = input_req
        self.ifRetrans = ifRetrans
        self.nRetrans = nRetrans
        self.bankName = int(self.bankName)
        self.account_no = self.account_no
        self.clientIP = self.clientIP
        self.requests = self.input_req
        self.ifRetrans = self.ifRetrans
        self.timeout = self.timeout
        self.nRetrans = self.nRetrans
        self.ifRandom = self.ifRandom
        self.master = self.master
        self.head_srvs = self.head_srvs
        self.tail_srvs = self.tail_srvs

    def main(self):
        self.output(((('Client: Bank Name is: ' + str(self.bankName)) + '  Account number is: ') + str(self.account_no)))
        reqList = self.init_req()
        num_req = len(reqList)
        for i in range(num_req):
            req = reqList[i]
            if (req.reqtype == ReqType.Query):
                dst = self.tail_srvs.get(self.bankName)
                print('DESTINATION the dst is', dst)
            else:
                dst = self.head_srvs.get(self.bankName)
                print('DESTINATION the dst is', dst)
            self.output((('Request ' + str(req.reqID)) + ' has been sent out'))
            time.sleep(2)
            clk = self.logical_clock()
            self._send(('REQ', req), dst)
            reply = rclk = dst = None

            def ExistentialOpExpr_1():
                nonlocal reply, rclk, dst
                for (_, (rclk, _, dst), (_ConstantPattern78_, reply)) in self._ClientReceivedEvent_5:
                    if (_ConstantPattern78_ == 'REPLY'):
                        if (rclk > clk):
                            return True
                return False
            _st_label_384 = 0
            self._timer_start()
            while (_st_label_384 == 0):
                _st_label_384 += 1
                if ExistentialOpExpr_1():
                    continue
                    _st_label_384 += 1
                elif self._timer_expired:
                    self.output('waiting for reply TIMEDOUT!')
                    if (req.reqtype == ReqType.Query):
                        dst = self.tail_srvs.get(self.bankName)
                    else:
                        dst = self.head_srvs.get(self.bankName)
                    print('KKKKKKKKKKKKKKKKKKKKKKKK', dst)
                    self.resend(req, dst)
                    _st_label_384 += 1
                else:
                    super()._label('_st_label_384', block=True, timeout=self.timeout)
                    _st_label_384 -= 1
            else:
                if (_st_label_384 != 2):
                    continue
            if (_st_label_384 != 2):
                break

    def init_req(self):
        reqList = []
        client_id = self.id
        if (self.ifRandom == True):
            input_str = self.input_req[0]
            strs = parse_randomReq(input_str)
            seed_num = strs[0]
            num_req = strs[1]
            prob_query = strs[2]
            prob_deposit = strs[3]
            prob_withdraw = strs[4]
            prob_transfer = strs[5]
            random.seed(seed_num)
            for i in range(num_req):
                a = random.randint(1, 100)
                a = float((a / 100))
                amount = (random.randint(1, 10) * 100)
                if (a < prob_query):
                    reqtype = ReqType.Query
                elif (a < (prob_query + prob_deposit)):
                    reqtype = ReqType.Deposit
                elif (a < ((prob_query + prob_deposit) + prob_withdraw)):
                    reqtype = ReqType.Withdraw
                else:
                    reqtype = ReqType.Transfer
                reqID = ((((str(self.bankName) + '.') + str(self.account_no[0])) + '.') + str((i + 1)))
                req = Request(reqID, reqtype, self.account_no, amount, client_id)
                self.output(('Generating randomized request: ' + str(req)))
                reqList.append(req)
        else:
            num_req = len(self.input_req)
            for i in range(num_req):
                strs = parse_req(self.input_req[i])
                if (strs[0].startswith(str(self.bankName)) and (int(strs[2]) == int(self.account_no))):
                    req = Request(strs[0], strs[1], strs[2], strs[3], client_id)
                    reqList.append(req)
                else:
                    pass
        return reqList

    def resend(self, req, dst):
        for i in range(self.nRetrans):
            self.output('Start retransmitting Request ', req.reqID)
            time.sleep(1)
            clk = self.logical_clock()
            self._send(('REQ', req), dst)
            dst = rclk = reply = None

            def ExistentialOpExpr_0():
                nonlocal dst, rclk, reply
                for (_, (rclk, _, dst), (_ConstantPattern64_, reply)) in self._ClientReceivedEvent_4:
                    if (_ConstantPattern64_ == 'REPLY'):
                        if (rclk > clk):
                            return True
                return False
            _st_label_364 = 0
            self._timer_start()
            while (_st_label_364 == 0):
                _st_label_364 += 1
                if ExistentialOpExpr_0():
                    break
                    _st_label_364 += 1
                elif self._timer_expired:
                    self.output('waiting for reply TIMEDOUT!')
                    continue
                    _st_label_364 += 1
                else:
                    super()._label('_st_label_364', block=True, timeout=self.timeout)
                    _st_label_364 -= 1
            else:
                if (_st_label_364 != 2):
                    continue
            if (_st_label_364 != 2):
                break
        self.output('Retransmit time equals the RetransLimit, stop retransmitting Request', req.reqID)

    def _Client_handler_8(self, reply, src_id):
        self.output(('Reply received from server: ' + str(reply)))
    _Client_handler_8._labels = None
    _Client_handler_8._notlabels = None

    def _Client_handler_9(self, master, newSrv):
        print('CLIENT ', self.id, 'CHANGING NEW HEAD: ', newSrv)
        self.head_srvs.update({self.bankName: newSrv})
    _Client_handler_9._labels = None
    _Client_handler_9._notlabels = None

    def _Client_handler_10(self, newSrv, master):
        print('SETTING NEW TAIL: ', newSrv)
        self.tail_srvs.update({self.bankName: newSrv})
    _Client_handler_10._labels = None
    _Client_handler_10._notlabels = None

    def _Client_handler_11(self, master, newSrv):
        print('CLIENT RECEIVED SERVER FAIL, STARTING TO SLEEP')
        time.sleep(5)
    _Client_handler_11._labels = None
    _Client_handler_11._notlabels = None

def main():
    srvDic = collections.defaultdict(list)
    cliDic = collections.defaultdict(list)
    waitList = []
    head_srvs = {}
    tail_srvs = {}
    da.api.config(channel='fifo', handling='all', clock='Lamport')
    flag_srv = False
    flag_cli = False
    flag_req = False
    flag_config = False
    input_srv = []
    input_cli = []
    input_req = []
    config_file = None
    if (len(sys.argv) < 2):
        print('Please input config file!')
        sys.exit()
    else:
        config_file = sys.argv[1]
    fp = open(config_file, 'r')
    for line in fp:
        if line.startswith('//Server'):
            flag_srv = True
            continue
        elif line.startswith('//Client '):
            flag_config = True
            input_str = fp.__next__()
            list_config = parse_config(input_str)
            continue
        elif line.startswith('//Client:'):
            flag_cli = True
            continue
        elif line.startswith('//Request'):
            flag_req = True
            continue
        elif ((flag_srv == True) and (flag_config == False)):
            input_srv.append(line)
        elif ((flag_cli == True) and (flag_req == False)):
            input_cli.append(line)
        elif ((flag_req == True) and (not (line == '\n'))):
            input_req.append(line)
        else:
            pass
    srvList = []
    for string in input_srv:
        srv = parse_server(string)
        srvList.append(srv)
    num_srv = len(input_srv)
    servers = list(da.api.new(Server, num=num_srv))
    bankName_srv = []
    serverIP = []
    startup_delay = []
    life_time = []
    for i in range(num_srv):
        bankName_srv.append(srvList[i][0])
        serverIP.append(srvList[i][1])
        startup_delay.append(srvList[i][2])
        life_time.append(srvList[i][3])
    for i in range(num_srv):
        if ((i == 0) or (not (bankName_srv[i] == bankName_srv[(i - 1)]))):
            prev = None
        else:
            prev = servers[(i - 1)]
        if ((i == (num_srv - 1)) or (not (bankName_srv[i] == bankName_srv[(i + 1)])) or (not (startup_delay[(i + 1)] == 0))):
            next = None
        else:
            next = servers[(i + 1)]
        if ((prev == None) and (startup_delay[i] == 0)):
            head_srvs.update({bankName_srv[i]: servers[i]})
        elif ((next == None) and (startup_delay[i] == 0)):
            tail_srvs.update({bankName_srv[i]: servers[i]})
        if (startup_delay[i] == 0):
            srvDic[bankName_srv[i]].append([servers[i], prev, next, serverIP[i]])
        else:
            waitList.append([bankName_srv[i], servers[i], prev, next, serverIP[i]])
    cliList = []
    for string in input_cli:
        cli = parse_client(string)
        cliList.append(cli)
    num_cli = len(input_cli)
    clients = list(da.api.new(Client, num=num_cli))
    bankName_cli = []
    account_no = []
    clientIP = []
    ifRetrans = list_config[0]
    timeout = list_config[1]
    nRetrans = list_config[2]
    ifRandom = list_config[3]
    for i in range(num_cli):
        bankName_cli.append(cliList[i][0])
        account_no.append(cliList[i][1])
        clientIP.append(cliList[i][2])
    for i in range(num_cli):
        cliDic[bankName_cli[i]].append(clients[i])
    print(cliDic)
    filename_master = '/Users/fydeng/Desktop/Distalgo/master_log.txt'
    master = da.api.new(Master)
    da.api.setup(master, (srvDic, cliDic, filename_master, waitList))
    da.api.start(master)
    for i in range(num_srv):
        if ((i == 0) or (not (bankName_srv[i] == bankName_srv[(i - 1)]))):
            prev = None
        else:
            prev = servers[(i - 1)]
        if ((i == (num_srv - 1)) or (not (bankName_srv[i] == bankName_srv[(i + 1)])) or (not (startup_delay[(i + 1)] == 0))):
            next = None
        else:
            next = servers[(i + 1)]
        da.api.setup(servers[i], (bankName_srv[i], serverIP[i], startup_delay[i], life_time[i], prev, next, master))
    da.api.start(servers)
    for i in range(num_cli):
        da.api.setup(clients[i], (bankName_cli[i], account_no[i], clientIP[i], input_req, ifRetrans, timeout, nRetrans, ifRandom, master, head_srvs, tail_srvs))
    da.api.start(clients)
