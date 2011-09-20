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

	void sendMessageA();
	void handleConnectedA();
	void handleDisconnectedA(const boost::optional<Swift::ClientError>&);
	void handleMessageReceivedA(boost::shared_ptr<Swift::Message>);
	void handleMessageTimeoutA();

	void sendMessageB();
	void handleConnectedB();
	void handleDisconnectedB(const boost::optional<Swift::ClientError>&);
	void handleMessageReceivedB(boost::shared_ptr<Swift::Message>);
	void handleMessageTimeoutB();

	void handleDataRead(const Swift::SafeByteArray&);

private:
	inline void checkIfDone();

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

	int connectedClients;

	Swift::IDGenerator idGenerator; // probably a bottleneck due to UUID usage;
	bool done;

	Swift::CoreClient* clientA;
	std::string messageHeaderA;
	std::string messageFooterA;

	std::list<MessageStamp> sendMessagesA;
	int noOfSendMessagesA;
	std::list<MessageStamp> receivedMessagesA;
	int noOfReceivedMessagesA;
	Swift::Timer::ref messageTimeoutA;

	Swift::CoreClient* clientB;
	std::string messageHeaderB;
	std::string messageFooterB;

	std::list<MessageStamp> sendMessagesB;
	int noOfSendMessagesB;
	std::list<MessageStamp> receivedMessagesB;
	int noOfReceivedMessagesB;
	Swift::Timer::ref messageTimeoutB;

	boost::posix_time::ptime begin;
	boost::posix_time::ptime end;
	boost::uintmax_t bytesReceived;
};
