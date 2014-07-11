CXX = clang++
CXXFLAGS = -O2 -W -Wall -std=c++11
LDFLAGS = -lpthread -lboost_system -lboost_thread -lcppnetlib-uri -lcppnetlib-client-connections -lssl -lcrypto
DEST = /usr/local/bin
SRC = src/http.cpp
PROGRAM = server

all: $(PROGRAM)

$(PROGRAM):; $(CXX) $(SRC) $(CXXFLAGS) $(LDFLAGS) -o $(PROGRAM)

clean:; rm -f *.o *~ $(PROGRAM) $(OUTFILE)

install: $(PROGRAM)
	     install -s $(PROGRAM) $(DEST)

uninstall:; rm -f $(DEST)/$(PROGRAM)

