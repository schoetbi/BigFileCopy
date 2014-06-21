
bfcp.o: bfcp.cpp
	g++ -c -o $@ $<

bfcp: bfcp.o
	g++ -o bfcp bfcp.o -lcrypto -lssl -lboost_filesystem -lboost_system


