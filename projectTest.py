import re
import time
import sys
from init import *
from random import *

head_srvs = {}
tail_srvs = {}
def parse_config(input_str):
    input_str = input_str.replace(' ','')
    strs = input_str.split(',')
    if int(strs[0])!= 0:
       	retrans = True
       	print ('retransmission enabled')
    else:
       	reTrans = False
    retrans_interval = int(strs[1])
    print ('retransmission interval is:',retrans_interval)
    retrans_time = int(strs[2])
    print ('retransmission time is:',retrans_time)
    if int(strs[3])!= 0:
    	ifRandom = True
    	print ("Random request enabled")
    else:
    	ifRandom = False
    return [retrans, retrans_interval, retrans_time, ifRandom]

def parse_server(input_str):
		input_str =input_str.strip()
		input_str =input_str.replace(' ','')
		str =input_str.split(',')
		bankName =str[0]
		startup_delay =str[2]
		life_time =str[3]
		return [bankName,startup_delay,life_time]

def parse_client(input_str):
		input_str = input_str.replace(' ','')
		str = input_str.split(',')
		bankName = str[0]
		account_no = str[1]
		return [bankName,account_no]

def parse_req(input_str):
    input_str = input_str.replace(' ','')
    input_str = re.sub('[()]','',input_str)
    strs = input_str.split(',')
    if(strs[0]=='getBalance'):
        reqtype = ReqType.Query
    elif(strs[0]=='deposit'):
        reqtype = ReqType.Deposit
    elif(strs[0]=='withdraw'):
        reqtype = ReqType.Withdraw
    else:
        reqtype = ReqType.Transfer
    reqID = strs[1]
    account_num = strs[2]
    if reqtype == ReqType.Query:
        amount = 0
    else:
        amount = strs[3]
    return [reqID,reqtype,account_num,amount]



class Server(process):
	def setup(bankName, startup_delay, life_time, prev, next):
		self.bankName = bankName
		self.startup_delay =startup_delay
		self.life_time = int(life_time)
		self.prev =prev
		self.next =next
		self.sentTrans = []
		self.procTrans = {}
		self.AccountInfo = {}
		#print(self.bankName, self.startup_delay, self.life_time, self.prev, self.id, self.next)

	def proc_balance(req):
		outcome = Outcome.Processed
		if self.AccountInfo.get(req.account_num) == None:
			cur_balance = 0
		else:
			cur_balance = AccountInfo[req.account_num]
		if procTrans.get(req.reqID) != None:
			if procTrans[req.reqID].reqtype != req.reqtype:
				print("inconsistent")
				outcome = Outcome.InconsistentWithHistory
			else:
				print("duplicate")
				pass
			new_balance = cur_balance
		else:
			if (req.reqtype == ReqType.Query):
				new_balance = cur_balance
			elif (req.reqtype == ReqType.Deposit):
				new_balance = cur_balance + req.amount
			elif (req.reqtype == ReqType.Withdraw):
				if (cur_balance - req.amount < 0):
					new_balance = cur_balance
					print("insufficient")
					outcome = Outcome.InsufficientFunds
				else:
					new_balance = cur_balance - req.amount;
		if req.reqtype != ReqType.Query:
			sentTrans.append(req)
		AccountInfo.update({req.account_num:new_balance})		
		return [new_balance, outcome]

	def update_procTrans(reqID):
		len_sentTrans = len(sentTrans)
		for i in range(len_sentTrans):
			if (sentTrans[i].reqID == reqID):
				req = sentTrans[i]
				sentTrans.pop(i)
				if req.reqtype == ReqType.Query:
					break
				proc_req = self.procTrans.get(req.reqID)
				if proc_req == None:
					self.procTrans.update({req.reqID:req})
					print("Request %s has been added to processed transaction" % req.reqID)
				elif req.reqtype == proc_req.reqtype:
					print("Duplicate request, donot add to processed transaction")
				else:
					print("Inconsistent With Histroy request, donot add to processed transaction")
				break

	def receive(msg=("REQ", req), from_= src_id):
		print("Request %s from client received " % req.reqID)
		if(self.life_time > 0):
			self.life_time = self.life_time-1
			print("the current life_time is: ",self.life_time)
		elif(self.life_time==0):
			print("Server",self.id,"has expired!")
			return		#which causes the server not sending reply back to the client
		result = proc_balance(req)
		new_balance = result[0]
		outcome = result[1]
		reply = Reply(req.reqID,outcome,new_balance)
		ack = Ack(req.reqID)
		if (prev == None):
			send(('REQ', req), to = next)
		elif (next == None):
			update_procTrans(req.reqID)
			send(('REPLY', reply), to = req.client_id)
			if (req.reqtype != ReqType.Query):
				send(('ACK', ack), to = prev)
				print(ack, "has been sent")
		else:
			send(('REQ', req), to = next)
		print("Reply for Request %s has been sent" % reply.reqID)

	def receive(msg=('ACK', ack), from_= prev):
		print(ack, "has been received")
		update_procTrans(ack.reqID)


	def main():
		print("Server: ", self.id, "Bank Name is: ", self.bankName, "Life time is: ", self.life_time, "Previous server is ", self.prev, "Next server is ", self.next)
		await(0)
		print("exit")

class Client(process):
	def setup(bankName,account_no,input_req,ifRetrans,timeout,nRetrans,ifRandom):
		self.bankName = bankName
		self.account_no = account_no
		self.requests = input_req
		self.ifRetrans = ifRetrans
		self.timeout = timeout
		self.nRetrans = nRetrans
		self.ifRandom = ifRandom
		#print(self.bankName, self.account_no)

	def init_req():
		reqList =[]
		client_id = self.id
		if ifRandom == True:
			random_list = input_req[0]
			seed_num = int(random_list[0])
			num_req = int(random_list[1])
			prob_query = float(random_list[2])
			prob_deposit = float(random_list[3])
			prob_withdraw = float(random_list[4])
			prob_transfer = float(random_list[5])
			a = Random()
			a = seed(seed_num)
			for i in range(num_req):
				b = float(a.randint(1, 100) / 100)
				if b < prob_query:
					reqtype = ReqType.Query
				elif b < (prob_query + prob_deposit):
					reqtype = ReqType.Deposit
				elif b < (prob_query + prob_deposit + prob_withdraw):
					reqtype = ReqType.Withdraw
				else:
					reqtype = ReqType.Transfer
				reqID = str(self.bankName) + str(self.client_id)



		else:
			num_req = len(input_req)
			for i in range(num_req):
				strs = parse_req(input_req[i])
				if strs[0].startswith(self.bankName) and int(strs[2]) == int(self.account_no):
					req = Request(strs[0], strs[1], strs[2], strs[3], client_id)
					reqList.append(req)
				else:
					pass
		return reqList

	def receive(msg=('REPLY', reply), from_= src_id):
		print("Reply received from server: ",reply)

	def main():
		print("Client: Bank Name is: ", self.bankName, "Account number is: ", self.account_no)
		reqList = init_req()
		num_req = len(reqList)
		for i in range(num_req):
			req = reqList[i]
			if req.reqtype == ReqType.Query:
				dst = tail_srvs.get(self.bankName)
			else:
				dst = head_srvs.get(self.bankName)
			#print(req)
			print("Request ", req.reqID, "has been sent out")
			print("destination is: ", dst)
			time.sleep(1)
			clk = logical_clock()
			send(('REQ', req), to = dst)
			if await(some(received(('REPLY', reply),from_= dst,clk=rclk),has=(rclk>clk))):
				continue
			elif timeout(self.timeout):
				print('waiting for reply TIMEDOUT!')
				print('Resending the request:',req.reqID)
				i = i-1

def main():
	config(channel='fifo',handling='all',clock='Lamport')
	flag_srv =False
	flag_cli =False
	flag_req =False
	flag_config =False
	input_srv =[]
	input_cli =[]
	input_req =[]
	if len(sys.argv) < 2:
		print('Please input config file!')
		sys.exit()
	else:
		config_file = sys.argv[1]

	fp = open(config_file,'r')
	for line in fp:
		if line.startswith('//Server'):
			flag_srv =True
			continue
		elif line.startswith('//Client '):
			flag_config =True
			input_str = fp.__next__()
			list_config = parse_config(input_str)
			continue
		elif line.startswith('//Client:'):
			flag_cli =True
			continue
		elif line.startswith('//Request'):
			flag_req =True
			continue
		elif flag_srv==True and flag_config==False:
			input_srv.append(line)
		elif flag_cli==True and flag_req==False:
			input_cli.append(line)
		elif flag_req==True and line!='\n':
			input_req.append(line)
		else:
			pass
	
	#######################Initiate Servers################################
	srvList =[]
	for string in input_srv:
		srv = parse_server(string)
		srvList.append(srv)

	num_srv = len(input_srv)
	servers = list(new(Server, num = num_srv))
	bankName_srv = []
	startup_delay = []
	life_time = []
	

	for i in range(num_srv):
		bankName_srv.append(srvList[i][0])
		startup_delay.append(srvList[i][1])
		life_time.append(srvList[i][2])

	for i in range(num_srv):
		if i == 0 or bankName_srv[i]!= bankName_srv[i-1]:
			prev = None
		else:
			prev = servers[i-1]
		if i == num_srv - 1 or bankName_srv[i]!= bankName_srv[i+1]:
			next = None
		else:
			next = servers[i+1]
		if prev == None:
			head_srvs.update({bankName_srv[i]:servers[i]})
		elif next == None:
			tail_srvs.update({bankName_srv[i]:servers[i]})
		setup(servers[i],(bankName_srv[i], startup_delay[i], life_time[i], prev, next))

	
	#print("head servers are ", head_srvs)
	#print("tail servers are ", tail_srvs)		
	start(servers)
	#######################################################################

	#######################Initiate Clients################################
	cliList =[]
	for string in input_cli:
		cli = parse_client(string)
		cliList.append(cli)

	num_cli =len(input_cli)
	clients = list(new(Client, num = num_cli))
	bankName_cli =[]
	account_no =[]
	ifRetrans = list_config[0]
	timeout = list_config[1]
	nRetrans = list_config[2]
	ifRandom = list_config[3]

	for i in range(num_cli):
		bankName_cli.append(cliList[i][0])
		account_no.append(cliList[i][1])

	for i in range(num_cli):
		setup(clients[i],(bankName_cli[i],account_no[i],input_req,ifRetrans,timeout,nRetrans,ifRandom))
	start(clients)
	#######################################################################


	