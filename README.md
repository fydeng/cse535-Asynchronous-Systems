async-system

============

Implemented chain replication of asynchronous system in DistAlgo(extension of Python)

============ INSTRUCTIONS ============

DistAlgo Version:
	1. Make sure you have python3 installed in your computer.
	2. Make sure that your current path has Makefile, init.da, project.da and test files.
	3. Create client and server folder, which is used for saving log files.
	3. Remember to clear log files after each time running it.
	4. Then compile init.da file using the following command:
		python3 -m da.compiler init.da
	5. Compiler project.da file using the following command:
		python3 -m da.compiler project.da
	6. run project.da file using the following command(remember to add the config file which you want to test):
		python3 -m da project.da (test case).txt

============ MAIN FILES ============

DistAlgo Version:
	We combine the client and server side code, which is in the project.da file.
	The path for project.da file is: /src/Disalgo/
	The init.da file is used for defining Request, Reply and Ack Class.
	The path for init.da file is: /src/Distalgo/
	The Makefile is used to clear log files each time after running it.
	The path for Makefile is /src/Distalgo
	The client and server folder are used for saving log files.
	The path for these two files are /src/Distalgo/client/, /src/Distalgo/server/

============ BUGS AND LIMITATIONS ============

Currently there’re no bugs found in our code. We tested all kinds of cases mentioned in the grading sheet and the testing results are good.

============ LANGUAGE COMPARISON ============

Size of the code: the current code size of C++ version is about 2500 lines including both head files, cpp files and some necessary comments; the current code size of DistAlgo version is about 700 lines including both initialisation file and main file, which is much less than the C++ version.

Time for initial version: we spent about one week for initial version in C++ and about ten days for initial version in DistAlgo.

Time for debugging: we spent about one week for debugging in C++ and about 3 days for debugging in DistAlgo. We spent a lot of time optimising the algorithm in synchronising sentTrans and procTrans between servers.

Readability of code: Both these two versions of code are organised in a reasonable way and  quite readable.

Similarity to pseudo-code: Our code is developed based on the pseudo code in phase1, but improved in some cases. For example, we didn’t consider multiple server failures in pseudo-code algorithm.

Strength of C++: 
We use sockets for network connection in C++ version,specifically use TCP for connection between servers and connection between master and server, use UDP for connection between servers and clients. In this way, we can detect server failure if the connection fails and stop to synchronise sentTrans between servers immediately, in which case we don’t need to process some unnecessary operation. However, there is no such protocol difference in DistAlgo(all is implemented in sent()), servers just send “SYNC” request to the next server regardless of the current network situation in server chain.

Strength of DistAlgo:
1. The send() function in DistAlgo allows us to send message to a specific destination(process) by just giving the process id. However, it’s much more complicated in C++ version, we have to pack the message and destination first before sending it out, which adds the size of the code.
2. When dealing with the situation that both clients and servers needs to receive different kinds of messages, in DistAlgo, we implemented this by await() for different situations to be true and then handles them separately. However, in C++, we have to create  new threads for each new message, which adds the size of code too.

============ OTHER COMMENTS ============

In order to facilitate testing in C++, in some cases where chain extension is not introduced, we still adds the new server in the configuration file by setting its startup-delay really big(which can be regarded as no server adding to the chain).In this way, the testing results are not affected at all.

We designed a test case specially for client message loss, in other cases, we assume that there’s no message loss during the operation.

