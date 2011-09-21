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

#include "AccountDataProvider.h"
#include "BenchmarkSession.h"

class ActiveSessionPair : public BenchmarkSession {
public:
	ActiveSessionPair(AccountDataProvider* accountDataProvider, Swift::NetworkFactories* networkFactories, Swift::CertificateTrustChecker* trustChecker, int warmUpMessages, int messages, std::string body, bool noCompression, bool noTLS);
	virtual ~ActiveSessionPair();

	virtual void start();
	virtual void stop();
	virtual void benchmark();

	virtual BenchmarkSession::LatencyInfo getLatencyResults();

private:
	void prepareMessageTemplate();

	void sendMessageFromAToB();
	void handleConnectedA();
	void handleDisconnectedA(const boost::optional<Swift::ClientError>&);
	void handleMessageReceivedByAFromB(boost::shared_ptr<Swift::Message>);
	void handleMessageTimeoutA();

	void sendMessageFromBToA();
	void handleConnectedB();
	void handleDisconnectedB(const boost::optional<Swift::ClientError>&);
	void handleMessageReceivedByBFromA(boost::shared_ptr<Swift::Message>);
	void handleMessageTimeoutB();

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
	int totalMessages;
	std::string body;
	bool noCompression;
	bool noTLS;

	int connectedClients;

	Swift::IDGenerator idGenerator; // probably a bottleneck due to UUID usage;
	bool benchmarkingStartedA;
	bool benchmarkingStartedB;
	bool benchmarkingDone;
	bool done;

	bool dataCountingForA;
	bool dataCountingForB;

	Swift::CoreClient* clientA;
	std::string messageHeaderA;
	std::string messageFooterA;

	std::list<MessageStamp> sendMessagesFromA;
	int noOfSendMessagesFromAToB;
	std::list<MessageStamp> receivedMessagesA;
	int noOfReceivedMessagesByAFromB;
	Swift::Timer::ref messageTimeoutA;

	Swift::CoreClient* clientB;
	std::string messageHeaderB;
	std::string messageFooterB;

	std::list<MessageStamp> sendMessagesFromB;
	int noOfSendMessagesFromBToA;
	std::list<MessageStamp> receivedMessagesB;
	int noOfReceivedMessagesByBFromA;
	Swift::Timer::ref messageTimeoutB;

	boost::posix_time::ptime begin;
	boost::posix_time::ptime end;
	boost::uintmax_t bytesReceived;
};
