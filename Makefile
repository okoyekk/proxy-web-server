WEBSERVER = web/webserver
PROXYSERVER = proxy/proxyserver

SRC1 = web/webserver.cc
SRC2 = proxy/proxyserver.cc

CXX = g++
CXXFLAGS = -std=c++11 -Wall

all: $(WEBSERVER) $(PROXYSERVER)

$(WEBSERVER): $(SRC1)
	$(CXX) $(CXXFLAGS) $(SRC1) -o $(WEBSERVER)

$(PROXYSERVER): $(SRC2)
	$(CXX) $(CXXFLAGS) $(SRC2) -o $(PROXYSERVER)

clean:
	rm -f $(WEBSERVER) $(PROXYSERVER)
