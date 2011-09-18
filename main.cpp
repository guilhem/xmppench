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
#include <fstream>
#include <vector>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include <Swiften/JID/JID.h>
#include <Swiften/EventLoop/SimpleEventLoop.h>
#include <Swiften/Network/BoostNetworkFactories.h>

#include "AccountDataProvider.h"
#include "LatencyWorkloadBenchmark.h"
#include "ThreadSafeNetworkFactories.h"

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
	WorkerLoop(const std::string& ip) : thread(0), eventLoop() {
		networkFactories = new ThreadSafeNetworkFactories(&eventLoop, ip);
		//networkFactories = new BoostNetworkFactories(&eventLoop);
		/*if (ip.empty()) {
			networkFactories = new BoostNetworkFactories(&eventLoop);
		} else {
			networkFactories = new ThreadSafeNetworkFactories(&eventLoop, ip);
		}*/
	}

	~WorkerLoop() {
		delete networkFactories;
		delete thread;
	}

	void start() {
		thread = new boost::thread(boost::bind(&WorkerLoop::run, this));
	}

	void wait() {
		if (thread) thread->join();
	}

	NetworkFactories* getNetworkFactories() {
		return networkFactories;
	}

public:
	void run() {
		eventLoop.run();
	}

private:
	std::string ip;
	boost::thread* thread;
	SimpleEventLoop eventLoop;
	NetworkFactories *networkFactories;
};

int main(int argc, char *argv[]) {
	std::string hostname;
	std::string ip;
	std::string bodyfile;
	bool waitAtBeginning;
	int jobs = 1;

	LatencyWorkloadBenchmark::Options options;

	po::options_description desc("Allowed options");
	desc.add_options()
			("actives", po::value<int>(&options.noOfActiveSessions)->default_value(2000),		"number of active connections")
			("bodyfile", po::value<std::string>(&bodyfile),									"file to read the (unchecked!) body message from")
			("help",																			"produce help message")
			("hostname", po::value<std::string>(&hostname)->default_value("localhost"),		"hostname of benchmarked server")
			("idles", po::value<int>(&options.noOfIdleSessions)->default_value(8000),			"number of idle connections")
			("ip", po::value<std::string>(&ip),												"specify the IP to connect to; overrides DNS lookups;"
																							" required with jobs > 1")
			("jobs", po::value<int>(&jobs)->default_value(1),									"number of threads to run ! EXPERIMENTAL !")
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

	if (jobs > 1 && ip.empty()) {
		std::cout << "Error: You have to specify the IP of the server (use --ip) if jobs is greater than 1." << std::endl;
		return 1;
	}
	if (jobs > 1) {
		std::cout << "Warning: Running multiple worker threads is an experimental feature." << std::endl;
	}

	if (!bodyfile.empty()) {
		std::ifstream inputStream(bodyfile.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

		std::ifstream::pos_type fSize = inputStream.tellg();
		inputStream.seekg(0, std::ios::beg);

		std::vector<char> bytes(fSize);
		inputStream.read(&bytes[0], fSize);

		options.bodymessage = std::string(&bytes[0], fSize);
	} else {
		options.bodymessage = "Hey. You forgot to specify a body message.";

	}

	std::vector<WorkerLoop*> workers;
	std::vector<NetworkFactories*> networkFactories;
	for (int n = 0; n < jobs - 1; ++n) {
		WorkerLoop *worker = new WorkerLoop(ip);
		workers.push_back(worker);
		networkFactories.push_back(worker->getNetworkFactories());
	}

	WorkerLoop foo(ip);
	networkFactories.push_back(foo.getNetworkFactories());

	ContinousAccountProivder accountProvider("localhost");

	LatencyWorkloadBenchmark benchmark(networkFactories, &accountProvider, options);

	for (int n = 0; n < jobs - 1; ++n) {
		workers[n]->start();
	}
	foo.run();
	/*

	while(!workers.empty()) {
		std::cout << "Foo" << std::endl;
		WorkerLoop *worker = workers.back();
		worker->wait();
		delete worker;
		workers.pop_back();
	}*/
	return 0;
}
