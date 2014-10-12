
import da
import re
from init import *

def parse_config(input_str):
    input_str = input_str.replace(' ', '')
    strs = input_str.split(',')
    if (not (int(strs[0]) == 0)):
        retrans = True
        print('retransmission enabled')
    else:
        reTrans = False
    retrans_interval = int(strs[1])
    print('retransmission interval is:', retrans_interval)
    retrans_time = int(strs[2])
    print('retransmission time is:', retrans_time)
    return [retrans, retrans_interval, retrans_time]

def parse_server(input_str):
    input_str = input_str.strip()
    input_str = input_str.replace(' ', '')
    str = input_str.split(',')
    bankName = str[0]
    startup_delay = str[2]
    life_time = str[3]
    return [bankName, startup_delay, life_time]

def parse_client(input_str):
    input_str = input_str.replace(' ', '')
    str = input_str.split(',')
    bankName = str[0]
    account_no = str[1]
    return [bankName, account_no]

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
    account_num = int(strs[2])
    if (reqtype == ReqType.Query):
        amount = None
        return [reqID, reqtype, account_num]
    else:
        amount = int(strs[3])
        return [reqID, reqtype, account_num, amount]

class Server(da.DistProcess):

    def __init__(self, parent, initq, channel, props):
        super().__init__(parent, initq, channel, props)
        self._events.extend([])

    def setup(self, bankName, startup_delay, life_time, prev, next):
        self.bankName = bankName
        self.startup_delay = startup_delay
        self.life_time = life_time
        self.prev = prev
        self.next = next
        self.bankName = self.bankName
        self.startup_delay = self.startup_delay
        self.life_time = self.life_time
        self.prev = self.prev
        self.next = self.next
        self.sentTrans = []
        self.procTrans = {}
        print(self.bankName, self.startup_delay, self.life_time, self.prev, self.id, self.next)

    def main(self):
        pass

class Client(da.DistProcess):

    def __init__(self, parent, initq, channel, props):
        super().__init__(parent, initq, channel, props)
        self._events.extend([])

    def setup(self, bankName, account_no, input_req, ifRetrans, timeout, nRetrans):
        self.input_req = input_req
        self.account_no = account_no
        self.ifRetrans = ifRetrans
        self.timeout = timeout
        self.bankName = bankName
        self.nRetrans = nRetrans
        self.bankName = self.bankName
        self.account_no = self.account_no
        self.requests = self.input_req
        self.ifRetrans = self.ifRetrans
        self.timeout = self.timeout
        self.nRetrans = self.nRetrans
        print(self.bankName, self.account_no, self.ifRetrans, self.timeout, self.nRetrans)

    def main(self):
        num_req = len(self.input_req)
        reqList = []
        for i in range(num_req):
            strs = parse_req(self.input_req[i])
            if (strs[0].startswith(self.bankName) and (int(strs[2]) == int(self.account_no))):
                reqList.append(strs)
            else:
                pass
        print('my bankname is: ', self.bankName, 'my account number is: ', self.account_no, 'my request list is ', reqList)

def main():
    da.api.config(channel='fifo', handling='all', clock='Lamport')
    flag_srv = False
    flag_cli = False
    flag_req = False
    flag_config = False
    input_srv = []
    input_cli = []
    input_req = []
    fp = open('config.txt', 'r')
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
    startup_delay = []
    life_time = []
    head_srvs = {}
    tail_srvs = {}
    for i in range(num_srv):
        bankName_srv.append(srvList[i][0])
        startup_delay.append(srvList[i][1])
        life_time.append(srvList[i][2])
    for i in range(num_srv):
        if ((i == 0) or (not (bankName_srv[i] == bankName_srv[(i - 1)]))):
            prev = None
        else:
            prev = servers[(i - 1)]
        if ((i == (num_srv - 1)) or (not (bankName_srv[i] == bankName_srv[(i + 1)]))):
            next = None
        else:
            next = servers[(i + 1)]
        if (prev == None):
            head_srvs.update({bankName_srv[i]: servers[i]})
        elif (next == None):
            tail_srvs.update({bankName_srv[i]: servers[i]})
        da.api.setup(servers[i], (bankName_srv[i], startup_delay[i], life_time[i], prev, next))
    print('head servers are ', head_srvs)
    print('tail servers are ', tail_srvs)
    da.api.start(servers)
    cliList = []
    for string in input_cli:
        cli = parse_client(string)
        cliList.append(cli)
    num_cli = len(input_cli)
    clients = list(da.api.new(Client, num=num_cli))
    bankName_cli = []
    account_no = []
    ifRetrans = list_config[0]
    timeout = list_config[1]
    nRetrans = list_config[2]
    for i in range(num_cli):
        bankName_cli.append(cliList[i][0])
        account_no.append(cliList[i][1])
    for i in range(num_cli):
        da.api.setup(clients[i], (bankName_cli[i], account_no[i], input_req, ifRetrans, timeout, nRetrans))
    da.api.start(clients)
