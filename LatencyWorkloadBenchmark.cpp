#include "LatencyWorkloadBenchmark.h"

#include "BenchmarkSession.h"
#include "ActiveSessionPair.h"
#include "IdleSession.h"

#include <boost/format.hpp>


LatencyWorkloadBenchmark::LatencyWorkloadBenchmark(Swift::NetworkFactories* networkFactories, AccountDataProvider* accountProvider, Options& opt) : networkFactories(networkFactories), accountProvider(accountProvider), opt(opt) {
	std::cout << "Creating sessions...";
	// create active sessions
	for (int i = 0; i < opt.noOfActiveSessions / 2; ++i) {
		ActiveSessionPair *activePair = new ActiveSessionPair(accountProvider, networkFactories, &trustChecker, 100, opt.stanzasPerConnection, "Hello there.");
		activePair->onReady.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkSessionReady, this, activePair));
		activePair->onDoneBenchmarking.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkSessionDone, this, activePair));
		activeSessionPairs.push_back(activePair);
		sessionsToActivate.push_back(activePair);
	}

	// create idle sessions
	for (int i = 0; i < opt.noOfIdleSessions; ++i) {
		IdleSession *idleSession = new IdleSession(accountProvider, networkFactories, &trustChecker);
		idleSession->onReady.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkSessionReady, this, idleSession));
		idleSession->onDoneBenchmarking.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkSessionDone, this, idleSession));
		idleSessions.push_back(idleSession);
		sessionsToActivate.push_back(idleSession);
	}

	std::cout << "done." << std::endl;
	std::cout << "Preparing sessions...";
	std::cout.flush();

	nextActivateSession = sessionsToActivate.begin();
	for (size_t n = 0; n < opt.parallelLogins && n < sessionsToActivate.size(); ++n) {
		(*nextActivateSession)->start();
		++nextActivateSession;
	}
}

LatencyWorkloadBenchmark::~LatencyWorkloadBenchmark() {

}

void LatencyWorkloadBenchmark::handleBenchmarkSessionReady(BenchmarkSession* session) {
	readySessions.push_back(session);
	if (readySessions.size() == sessionsToActivate.size()) {
		std::cout << "done." << std::endl;
		std::cout << "All sessions ready. Starting benchmark now!" << std::endl;
		benchmark();
	} else {
		if (nextActivateSession != sessionsToActivate.end()) {
			(*nextActivateSession)->start();
			nextActivateSession++;
		}
	}
}

void LatencyWorkloadBenchmark::handleBenchmarkSessionDone(BenchmarkSession *session) {
	doneSessions.push_back(session);
	if (dynamic_cast<ActiveSessionPair*>(session)) {
		std::cout << "Active session done: " << doneSessions.size() << " / " << readySessions.size() << std::endl;
	}
	if (doneSessions.size() == readySessions.size()) {
		finishSessions();
	}
}

void LatencyWorkloadBenchmark::benchmark() {
	for(std::vector<BenchmarkSession*>::iterator i = readySessions.begin(); i != readySessions.end(); ++i) {
		BenchmarkSession* session = *i;
		session->benchmark();
	}
}

std::string timeToString(double seconds) {
	double microseconds = seconds * 1000 * 1000;
	static const char *siPrefix[] = {"µs", "ms", "s", NULL};
	int power = 0;
	while (microseconds >= 1000) {
		++power;
		microseconds = microseconds / 1000.0;
	}
	return str( boost::format("%.3lf %s") % microseconds % siPrefix[power] );
}

void LatencyWorkloadBenchmark::finishSessions() {
	std::cout << "Calculating results...";

	if (doneSessions.empty()) {
		exit(0);
	}

	std::list<BenchmarkSession*>::iterator i = doneSessions.begin();
	BenchmarkSession::LatencyInfo accumulated = (*i)->getLatencyResults();
	++i;
	for(; i != doneSessions.end(); ++i) {
		BenchmarkSession* session = *i;

		BenchmarkSession::LatencyInfo latency = session->getLatencyResults();
		if (latency.minSeconds < accumulated.minSeconds) accumulated.minSeconds = latency.minSeconds;
		if (latency.maxSeconds > accumulated.maxSeconds) accumulated.maxSeconds = latency.maxSeconds;
		accumulated.avgSeconds = (accumulated.avgSeconds + latency.avgSeconds) / 2;
		accumulated.stanzas += latency.stanzas;
	}
	std::cout << "done" << std::endl;

	std::cout << "Finishing sessions." << std::endl;
	for(std::vector<BenchmarkSession*>::iterator i = readySessions.begin(); i != readySessions.end(); ++i) {
		BenchmarkSession* session = *i;

		session->stop();
	}


	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "- Results -" << std::endl;
	std::cout << "No. of stanzas:  " << accumulated.stanzas << std::endl;
	std::cout << "Minimal latency: " << timeToString(accumulated.minSeconds) << std::endl;
	std::cout << "Average latency: " << timeToString(accumulated.avgSeconds) << std::endl;
	std::cout << "Maximal latency: " << timeToString(accumulated.maxSeconds) << std::endl;
	std::cout << std::endl;

	exit(0);
}
