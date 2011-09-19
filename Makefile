# xmppech - Benchmarking Tool for XMPP servers
#
#

SWIFTEN_WD=/home/tobias/Development/swift
SWIFTEN_INCLUDE=$(SWIFTEN_WD)
BOOST_INCLUDE=$(SWIFTEN_WD)/3rdParty/Boost/src

SWIFTEN_LDFLAGS=-L$(SWIFTEN_WD)/Swiften -lSwiften
BOOST_SWIFTEN_LDFLAGS=-L$(SWIFTEN_WD)/3rdParty/Boost -lSwiften_Boost
LIBIDN_SWIFTEN_LDFLAGS=-L$(SWIFTEN_WD)/3rdParty/LibIDN -lSwiften_IDN

LDFLAGS=$(SWIFTEN_LDFLAGS) $(BOOST_SWIFTEN_LDFLAGS)  $(LIBIDN_SWIFTEN_LDFLAGS) -lssl -lcrypto -lxml2 -lresolv -pthread
CFLAGS=-I$(SWIFTEN_INCLUDE) -I$(BOOST_INCLUDE)
CXX=g++

all:
	$(CXX) *.cpp $(CFLAGS) $(LDFLAGS) -o xmppench
