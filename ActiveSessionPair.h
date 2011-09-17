#pragma once

#include <boost/thread/thread.hpp>
#include "DateTime.h"

#include <Swiften/TLS/BlindCertificateTrustChecker.h>
#include <Swiften/Client/CoreClient.h>
#include <Swiften/Client/ClientError.h>
#include <Swiften/Base/IDGenerator.h>
#include <Swiften/Elements/Message.h>
#include <Swiften/Network/Timer.h>

#include "AccountDataProvider.h"
#include "BenchmarkSession.h"

class ActiveSessionPair : public BenchmarkSession {
public:
	ActiveSessionPair(AccountDataProvider* accountDataProvider, Swift::NetworkFactories* networkFactories, Swift::CertificateTrustChecker* trustChecker, int messagesPerSecond, int messages, std::string body);
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

private:
	inline void checkIfDone();

private:
	struct MessageStamp {
		MessageStamp(const std::string& text) : text(text) {}
		MessageStamp(const Swift::Message::ref msg) {
			text = msg->getSubject();
		}

		Time timestamp;
		std::string text;
	};

	void calculateLatencies(std::list<MessageStamp>& sent, std::list<MessageStamp>& received, std::vector<double>& latencies);

private:
	AccountDataProvider* accountDataProvider;
	Swift::NetworkFactories* networkFactories;
	Swift::CertificateTrustChecker* trustChecker;
	int messagesPerSecond;
	int messages;
	std::string body;

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
};
