# Makefile for echo client and server

CXX=			g++ $(CCFLAGS)

SERVER=		src/server.cc src/server.h src/main-server.cc
CLIENT=		src/client.cc src/client.h src/main-client.cc
OBJS =			$(ECHO-SERVER) $(ECHO-CLIENT)



CCFLAGS= -g -std=c++11 -w

all:	server client

server:
	$(CXX) -o bin/server $(SERVER)

client:$(ECHO-CLIENT)
	$(CXX) -o bin/client $(CLIENT)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) bin/server bin/client



# These lines ensure that dependencies are handled automatically.
#%.d:	%.cc
#	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $< \
#		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
#		[ -s $@ ] || rm -f $@'

#include	$(OBJS:.o=.d)
