async-system
============

Implemented chain replication of asynchronous system in DistAlgo(extension of Python)

============ INSTRUCTIONS ============

DistAlgo Version:
	1. Make sure you have python3 installed in your computer.
	2. Make sure that your current path has init.da, project.da and config.txt file.
	3. First compile init.da file using the following command:
		python3 -m da.compiler init.da
	4. Compiler project.da file using the following command:
		python3 -m da.compiler project.da
	5. run project.da file using the following command(remember to add the config file which you want to test):
		python3 -m da project.da config_.txt

============ MAIN FILES ============

DistAlgo Version:
	We combine the client and server side code, which is in the project.da file.
	The path for project.da file is: /src/Disalgo/
	The init.da file is used for Defining Request and Reply Class.
	The path for init.da file is: /src/Distalgo/

============ BUGS AND LIMITATIONS ============

DistAlgo Version:
	Since processes are not started in the same order with that in the config file, so the output in DistAlgo is not well-ordered.

============ LANGUAGE COMPARISON ============

DistAlgo is much simpler compared to C++ in the following ways:

1. Network Connections: In C++, we have to do network connections using sockets, each server and client is identified with their unique <IP Address, Port No>, while in DistAlgo, we never should worry about network connection because messages are sent and received between process.
2. Thread safety: I/O in DistAlgo is thread-safe, it has already solved possible thread conflits within the language.However, in C++, the language itself is not thread-safe, but we solved this problem by adding a thread-safe function: readline.c
