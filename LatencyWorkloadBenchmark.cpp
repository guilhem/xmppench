/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#include "LatencyWorkloadBenchmark.h"

#include <boost/format.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>

#include "ActiveSessionPair.h"
#include "BenchmarkSession.h"
#include "IdleSession.h"

LatencyWorkloadBenchmark::LatencyWorkloadBenchmark(std::vector<Swift::NetworkFactories*> networkFactories, AccountDataProvider* accountProvider, Options& opt) : networkFactories(networkFactories), accountProvider(accountProvider), opt(opt) {
	std::cout << "Creating sessions...";

	// create active sessions
	for (int i = 0; i < opt.noOfActiveSessions / 2; ++i) {
		ActiveSessionPair *activePair = new ActiveSessionPair(accountProvider, networkFactories[i % networkFactories.size()], &trustChecker, opt.warmupStanzas, opt.stanzasPerConnection, opt.bodymessage, opt.noCompression, opt.noTLS, opt.boshURL);
		activePair->onReady.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkSessionReady, this, activePair));
		activePair->onDoneBenchmarking.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkSessionDone, this, activePair));
		activePair->onBenchmarkStart.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkBegin, this));
		activePair->onBenchmarkEnd.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkEnd, this));
		activeSessionPairs.push_back(activePair);
		sessionsToActivate.push_back(activePair);
	}

	// create idle sessions
	for (int i = 0; i < opt.noOfIdleSessions; ++i) {
		IdleSession *idleSession = new IdleSession(accountProvider, networkFactories[i % networkFactories.size()], &trustChecker, opt.noCompression, opt.noTLS, opt.boshURL);
		idleSession->onReady.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkSessionReady, this, idleSession));
		idleSession->onDoneBenchmarking.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkSessionDone, this, idleSession));
		idleSessions.push_back(idleSession);
		sessionsToActivate.push_back(idleSession);
	}

	std::cout << "done." << std::endl;
	std::cout << "Preparing sessions...";
	std::cout.flush();

	nextActivateSession = sessionsToActivate.begin();
	for (size_t n = 0; n < static_cast<size_t>(opt.parallelLogins) && n < sessionsToActivate.size(); ++n) {
		(*nextActivateSession)->start();
		++nextActivateSession;
	}
}

LatencyWorkloadBenchmark::~LatencyWorkloadBenchmark() {

}

void LatencyWorkloadBenchmark::handleBenchmarkSessionReady(BenchmarkSession* session) {
	boost::unique_lock<boost::mutex> lock(handleSessionReadyMutex);
	readySessions.push_back(session);
	if (readySessions.size() == sessionsToActivate.size()) {
		std::cout << "done." << std::endl;
		std::cout << "All sessions ready. Starting benchmark now!" << std::endl;
		benchmark();
	} else {
		if (nextActivateSession != sessionsToActivate.end()) {
			(*nextActivateSession)->start();
			++nextActivateSession;
		}
	}
}

void LatencyWorkloadBenchmark::handleBenchmarkSessionDone(BenchmarkSession *session) {
	boost::unique_lock<boost::mutex> lock(handleSessionDoneMutex);
	doneSessions.push_back(session);
	if (dynamic_cast<ActiveSessionPair*>(session)) {
	//	std::cout << "Active session done: " << doneSessions.size() << " / " << readySessions.size() << std::endl;
	}
	if (doneSessions.size() == readySessions.size()) {
		finishSessions();
	}
}

void LatencyWorkloadBenchmark::handleBenchmarkSessionStopped(BenchmarkSession* session) {
	boost::unique_lock<boost::mutex> locak(handleSessionStoppedMutex);
	yetToBeStoppedSessions.erase(session);
	if (yetToBeStoppedSessions.empty()) {
		std::cout << "done." << std::endl;
		exit(0);
	}
}

void LatencyWorkloadBenchmark::handleBenchmarkBegin() {
	if (begin.is_not_a_date_time()) {
		begin = boost::posix_time::microsec_clock::local_time();
	}
}

void LatencyWorkloadBenchmark::handleBenchmarkEnd() {
	end = boost::posix_time::microsec_clock::local_time();
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

std::string speedToString(double speed, std::string unit) {
	static const char *siPrefix[] = {"", "k", "M", "G", "T", NULL};
	int power = 0;

	while (speed >= 1000) {
		++power;
		speed = speed / 1000.0;
	}

	return str( boost::format("%.3lf %s%s") % speed % siPrefix[power] % unit );

}

void LatencyWorkloadBenchmark::finishSessions() {
	boost::posix_time::time_duration benchmarkDuration = end - begin;
	std::cout << "Calculating results...";

	if (doneSessions.empty()) {
		exit(0);
	}

	std::vector<ActiveSessionPair*>::iterator i = activeSessionPairs.begin();
	BenchmarkSession::LatencyInfo accumulated = (*i)->getLatencyResults();
	++i;
	for(; i != activeSessionPairs.end(); ++i) {
		ActiveSessionPair* session = *i;

		BenchmarkSession::LatencyInfo latency = session->getLatencyResults();
		if (latency.minSeconds < accumulated.minSeconds) accumulated.minSeconds = latency.minSeconds;
		if (latency.maxSeconds > accumulated.maxSeconds) accumulated.maxSeconds = latency.maxSeconds;
		accumulated.avgSeconds += latency.avgSeconds;
		accumulated.receivedBytes += latency.receivedBytes;
		accumulated.stanzas += latency.stanzas;
		accumulated.latencies.insert(accumulated.latencies.end(), latency.latencies.begin(), latency.latencies.end());

		// helper code
		accumulated.sum += latency.sum;
		accumulated.sumOfSquared += latency.sumOfSquared;
	}
	double duration = static_cast<double>(benchmarkDuration.total_microseconds())/1000/1000;
	accumulated.avgSeconds /= activeSessionPairs.size();
	accumulated.bytesPerSecond = accumulated.receivedBytes / duration;
	accumulated.stanzasPerSecond = accumulated.stanzas / duration;

	double stddev = 0;
	for (std::vector<double>::iterator i = accumulated.latencies.begin(); i != accumulated.latencies.end(); ++i) {
		*i = (*i - accumulated.avgSeconds) * (*i - accumulated.avgSeconds);
		stddev += *i;
	}
	stddev /= accumulated.latencies.size();
	stddev = sqrt(stddev);

	double stddev_alt = 0;

	stddev_alt = sqrt( (accumulated.sumOfSquared - accumulated.sum * accumulated.sum / accumulated.stanzas) / accumulated.stanzas );
	std::cout << "done." << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "= xmppench =" << std::endl << std::endl;

	std::cout << "- Configuration -" << std::endl;
	std::cout << "Number of Jobs:         " << networkFactories.size() << std::endl;
	std::cout << "Active Connections:     " << opt.noOfActiveSessions << std::endl;
	std::cout << "Idle Connections:       " << opt.noOfIdleSessions << std::endl;
	std::cout << "Stanzas per Connection: " << opt.stanzasPerConnection << std::endl;
	std::cout << "Body Message Size:      " << speedToString(opt.bodymessage.size(), "Bytes") << std::endl;
	std::cout << "Stream Compression:     " << (opt.noCompression ? "Not Used" : "Used If Available") << std::endl;
	std::cout << "TLS:                    " << (opt.noTLS ? "Not Used" : "Used If Available") << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "- Results -" << std::endl;
	std::cout << "No. of stanzas:      " << accumulated.stanzas << std::endl;
	std::cout << "Duration:            " << boost::posix_time::to_simple_string(benchmarkDuration) << std::endl;
	std::cout << "Minimal latency:     " << timeToString(accumulated.minSeconds) << std::endl;
	std::cout << "Average latency:     " << timeToString(accumulated.avgSeconds) << std::endl;
	std::cout << "Maximal latency:     " << timeToString(accumulated.maxSeconds) << std::endl;
	std::cout << "Standard deviation:  " << timeToString(stddev) << std::endl;
	//std::cout << "Stadnard dev. (alt): " << timeToString(stddev_alt) << std::endl;
	std::cout << std::endl;
	std::cout << "Throughput (Stanza): " << speedToString(accumulated.stanzasPerSecond, "Stanzas/Second") << std::endl;
	std::cout << "Throughput (Data):   " << speedToString(accumulated.bytesPerSecond, "Bytes/Second") << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "Finishing sessions...";
	std::cout.flush();
	for(std::vector<BenchmarkSession*>::iterator i = readySessions.begin(); i != readySessions.end(); ++i) {
		BenchmarkSession* session = *i;
		session->onStopped.connect(boost::bind(&LatencyWorkloadBenchmark::handleBenchmarkSessionStopped, this, session));
		yetToBeStoppedSessions.insert(session);
		session->stop();
	}
}
