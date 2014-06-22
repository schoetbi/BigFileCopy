.PHONY: runtest

bfcp: bfcp.o
	g++ -o bfcp bfcp.o -lcrypto -lssl -lboost_filesystem -lboost_system

%.o: %.cpp
	g++ -c -o $@ $<

checksumfile.o: checksumfile.cpp
	g++ -c -o $@ $<

test: checksumfile.o test_main.o
	g++ -o $@ $? -lcrypto -lssl -lboost_filesystem -lboost_system

runtest: test
	./test
