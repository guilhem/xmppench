// Scenario:

//> Thanks for the discussion. Here are the scenarios that we would be
//> interested in:
//>
//> a) 10K senders/receivers with 20% of them transmitting simulataneously -
//> get average response time/latency, throughput.
//>
//> b) How many senders/receivers transmitting simulataneously will it take to
//> saturate a server running on a 2 CPU box while keeping average latency <
//50
//> milli secs.
//>
//> We are not as concerned with the roster list of the senders.
//> Dave Cridland @ 15:05
//That's what we've been asked for. 15:05
//I have no clue how latency's going to hit 50ms without the box having gone into swap, mind. :-)



#include <iostream>
#include <vector>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include <Swiften/JID/JID.h>
#include <Swiften/EventLoop/SimpleEventLoop.h>
#include <Swiften/Network/BoostNetworkFactories.h>

#include "AccountDataProvider.h"
#include "LatencyWorkloadBenchmark.h"

using namespace Swift;
namespace po = boost::program_options;

class ContinousAccountProivder : public AccountDataProvider {
public:
	ContinousAccountProivder(std::string hostname) : hostname(hostname), counter(0) { }

	virtual Account getAccount() {
		std::string jid  = boost::str( boost::format("%1%@%2%") % counter % hostname );
		std::string password = boost::str( boost::format("%1%") % counter);
		++counter;

		AccountDataProvider::Account acc;
		acc.jid = jid;
		acc.password = password;

		return acc;
	}

private:
	std::string hostname;
	unsigned long counter;
};

class WorkerLoop {
public:
	WorkerLoop() : thread(0), eventLoop(), networkFactories(&eventLoop) { }

	~WorkerLoop() {
		if (thread) thread->join();
		delete thread;
	}

	void start() {
		thread = new boost::thread(boost::bind(&WorkerLoop::run, this));
	}

	NetworkFactories* getNetworkFactories() {
		return &networkFactories;
	}

private:
	void run() {
		eventLoop.run();
	}

private:
	boost::thread* thread;
	SimpleEventLoop eventLoop;
	BoostNetworkFactories networkFactories;
};

int main(int argc, char *argv[]) {
	std::string hostname;
	std::string bodyfile;
	bool waitAtBeginning;
	int jobs = 1;

	LatencyWorkloadBenchmark::Options options;

	po::options_description desc("Allowed options");
	desc.add_options()
			("actives", po::value<int>(&options.noOfActiveSessions)->default_value(2000),		"number of active connections")
			("bodyfile", po::value<std::string>(&bodyfile),									"file to read the body message from")
			("help",																			"produce help message")
			("hostname", po::value<std::string>(&hostname)->default_value("localhost"),		"hostname of benchmarked server")
			("idles", po::value<int>(&options.noOfIdleSessions)->default_value(8000),			"number of idle connections")
			//("jobs", po::value<int>(&jobs)->default_value(1),									"number of threads to run")
			("plogins", po::value<int>(&options.parallelLogins)->default_value(2),			"number of parallel logins")
			("stanzas", po::value<int>(&options.stanzasPerConnection)->default_value(1000),	"stanzas to send per connection")
			("waitatstart", po::value<bool>(&waitAtBeginning),									"waits at the beginning on keyboard input")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << "\n";
		return 1;
	}

	if (waitAtBeginning) {
		char c;
		std::cin >> c;
	}

	std::vector<WorkerLoop*> workers;
	std::vector<NetworkFactories*> networkFactories;
	for (int n = 0; n < jobs; ++n) {
		WorkerLoop *worker = new WorkerLoop;
		workers.push_back(worker);
		networkFactories.push_back(worker->getNetworkFactories());
	}

	ContinousAccountProivder accountProvider("localhost");

	LatencyWorkloadBenchmark benchmark(networkFactories, &accountProvider, options);

	for (int n = 0; n < jobs; ++n) {
		workers[n]->start();
	}
	for (int n = 0; n < jobs; ++n) {
		delete workers[n];
	}
	return 0;
}