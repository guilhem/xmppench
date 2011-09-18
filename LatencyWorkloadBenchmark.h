#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>

#include <Swiften/Client/Client.h>
#include <Swiften/JID/JID.h>
#include <Swiften/Network/NetworkFactories.h>
#include <Swiften/TLS/BlindCertificateTrustChecker.h>

#include "AccountDataProvider.h"
#include "ActiveSessionPair.h"
#include "BenchmarkSession.h"
#include "IdleSession.h"

class LatencyWorkloadBenchmark {
public:
	struct Options {
		int noOfIdleSessions;
		int noOfActiveSessions;
		int stanzasPerConnection;
		int parallelLogins;
		std::string bodymessage;
	};

public:
	LatencyWorkloadBenchmark(std::vector<Swift::NetworkFactories*> networkFactories, AccountDataProvider* accountProvider, Options& opt);
	~LatencyWorkloadBenchmark();

private:
	void handleBenchmarkSessionReady(BenchmarkSession*);
	void handleBenchmarkSessionDone(BenchmarkSession*);

private:
	void benchmark();
	void finishSessions();

private:
	std::vector<Swift::NetworkFactories*> networkFactories;
	AccountDataProvider* accountProvider;
	Options opt;

	Swift::BlindCertificateTrustChecker trustChecker;


	std::vector<ActiveSessionPair*> activeSessionPairs;
	std::vector<IdleSession*> idleSessions;

	std::vector<BenchmarkSession*>::iterator nextActivateSession;
	std::vector<BenchmarkSession*> sessionsToActivate;
	std::vector<BenchmarkSession*> readySessions;
	std::list<BenchmarkSession*> doneSessions;

	boost::mutex handleSessionReadyMutex;
	boost::mutex handleSessionDoneMutex;
};
