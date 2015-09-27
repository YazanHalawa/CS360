# Makefile for simple echo client and server

CXX=		g++ $(CCFLAGS)
SERVER_MAIN=	msgd_main.o msgd.o Handler.o Buffer.o
CLIENT_MAIN=	msg_main.o msg.o 
OBJS =		$(SERVER_MAIN) $(CLIENT_Main)

LIBS=

CCFLAGS= -g

all:	msgd msg 

msgd: $(SERVER_MAIN)
	$(CXX) -o msgd $(SERVER_MAIN) $(LIBS)

msg: $(CLIENT_MAIN)
	$(CXX) -o msg $(CLIENT_MAIN) $(LIBS)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d)

realclean:
	rm -f $(OBJS) $(OBJS:.o=.d) msgd msg


# These lines ensure that dependencies are handled automatically.
%.d:	%.cc
	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

include	$(OBJS:.o=.d)
