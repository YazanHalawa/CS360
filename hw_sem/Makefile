# Makefile for simple echo client and server

CXX  =		g++ $(CCFLAGS)
MAIN =		pthreads-main.o thdata.o
OBJS =		$(MAIN)

LIBS = -lrt

CCFLAGS= -g -pthread

all:	main

main: $(MAIN)
	$(CXX) -o main $(MAIN) $(LIBS)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d)

realclean:
	rm -f $(OBJS) $(OBJS:.o=.d) main


# These lines ensure that dependencies are handled automatically.
%.d:	%.cc
	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

include	$(OBJS:.o=.d)
