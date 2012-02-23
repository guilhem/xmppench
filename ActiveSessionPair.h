/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#pragma once

#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/thread/thread.hpp>

#include <Swiften/Base/IDGenerator.h>
#include <Swiften/Client/ClientError.h>
#include <Swiften/Client/CoreClient.h>
#include <Swiften/Elements/Message.h>
#include <Swiften/Network/Timer.h>
#include <Swiften/TLS/BlindCertificateTrustChecker.h>
#include <Swiften/Base/URL.h>

#include "AccountDataProvider.h"
#include "BenchmarkSession.h"

class ActiveSessionPair : public BenchmarkSession {
public:
	ActiveSessionPair(AccountDataProvider* accountDataProvider, Swift::NetworkFactories* networkFactories, Swift::CertificateTrustChecker* trustChecker, int warmUpMessages, int messages, std::string body, bool noCompression, bool noTLS, const Swift::URL& boshURL);
	virtual ~ActiveSessionPair();

	virtual void start();
	virtual void stop();
	virtual void warmUp();
	virtual void benchmark(const boost::posix_time::ptime& now);

	virtual BenchmarkSession::LatencyInfo getLatencyResults();

	boost::signal<void ()> onReadyToWarmUp;

private:
	void prepareMessageTemplate();

	void sendMessage(int connection);
	void handleConnected(int connection);
	void handleDisconnected(int connection, const boost::optional<Swift::ClientError>&);
	void handleMessageReceived(int connection, boost::shared_ptr<Swift::Message>);
	void handleMessageTimeout(int connection);

	void handleDataRead(const Swift::SafeByteArray&);

private:
	inline void checkIfDone();
	inline void checkIfDoneBenchmarking();

private:
	struct MessageStamp {
		MessageStamp(const std::string& text) : text(text), timestamp(boost::posix_time::microsec_clock::local_time()) {}
		MessageStamp(const Swift::Message::ref msg) : timestamp(boost::posix_time::microsec_clock::local_time()) {
			text = msg->getSubject();
		}

		std::string text;
		boost::posix_time::ptime timestamp;
	};

	void calculateLatencies(std::list<MessageStamp>& sent, std::list<MessageStamp>& received, std::vector<double>& latencies);

private:
	AccountDataProvider* accountDataProvider;
	Swift::NetworkFactories* networkFactories;
	Swift::CertificateTrustChecker* trustChecker;
	int warmUpMessages;
	int messages;
	std::string body;
	bool noCompression;
	bool noTLS;
	bool benchmarking;

	int connectedClients;

	Swift::IDGenerator idGenerator; // probably a bottleneck due to UUID usage;
	bool benchmarkingDone;
	bool done;

	bool dataCounting[2];

	Swift::CoreClient* client[2];
	std::string messageHeader[2];
	std::string messageFooter[2];

	std::list<MessageStamp> sentMessages[2];
	int noOfSentMessages[2];
	std::list<MessageStamp> receivedMessages[2];
	int noOfReceivedMessages[2];
	Swift::Timer::ref messageTimeout[2];

	boost::posix_time::ptime begin;
	boost::posix_time::ptime end;
	boost::uintmax_t bytesReceived;
	
	Swift::URL boshURL;
};
