/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#pragma once

#include <vector>
#include <set>
#include <list>

#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
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
		int warmupStanzas;
		int parallelLogins;
		std::string bodymessage;
		bool noTLS;
		bool noCompression;
	};

public:
	LatencyWorkloadBenchmark(std::vector<Swift::NetworkFactories*> networkFactories, AccountDataProvider* accountProvider, Options& opt);
	~LatencyWorkloadBenchmark();

private:
	void handleBenchmarkSessionReady(BenchmarkSession*);
	void handleBenchmarkSessionDone(BenchmarkSession*);
	void handleBenchmarkSessionStopped(BenchmarkSession*);
	void handleBenchmarkEnd();

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
	std::list<BenchmarkSession*> doneSessions;
	std::set<BenchmarkSession*> yetToBeStoppedSessions;

	boost::mutex handleSessionReadyMutex;
	boost::mutex handleSessionDoneMutex;
	boost::mutex handleSessionStoppedMutex;

	boost::posix_time::ptime begin;
	boost::posix_time::ptime end;

	int sessionsReadyToBenchmark;
};
