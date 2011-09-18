#include "ActiveSessionPair.h"

#include <Swiften/Base/SafeByteArray.h>
#include <Swiften/Network/NetworkFactories.h>
#include <Swiften/TLS/CertificateTrustChecker.h>
#include <Swiften/Elements/Message.h>
#include <Swiften/Network/TimerFactory.h>
#include <algorithm>

using namespace Swift;

ActiveSessionPair::ActiveSessionPair(AccountDataProvider* accountDataProvider, Swift::NetworkFactories* networkFactories, Swift::CertificateTrustChecker* trustChecker, int messagesPerSecond, int messages, std::string body) :
	accountDataProvider(accountDataProvider), networkFactories(networkFactories), trustChecker(trustChecker), messagesPerSecond(messagesPerSecond), messages(messages), body(body), connectedClients(0), noOfSendMessagesA(0), noOfReceivedMessagesA(0), noOfSendMessagesB(0), noOfReceivedMessagesB(0), bytesReceived(0) {

	AccountDataProvider::Account accA = accountDataProvider->getAccount();
	AccountDataProvider::Account accB = accountDataProvider->getAccount();

	clientA = new CoreClient(Swift::JID(accA.jid), createSafeByteArray(accA.password), networkFactories);
	clientA->setCertificateTrustChecker(trustChecker);
	clientA->onConnected.connect(boost::bind(&ActiveSessionPair::handleConnectedA, this));
	clientA->onDisconnected.connect(boost::bind(&ActiveSessionPair::handleDisconnectedB, this, _1));


	clientB = new CoreClient(Swift::JID(accB.jid), createSafeByteArray(accB.password), networkFactories);
	clientB->setCertificateTrustChecker(trustChecker);
	clientB->onConnected.connect(boost::bind(&ActiveSessionPair::handleConnectedB, this));
	clientB->onDisconnected.connect(boost::bind(&ActiveSessionPair::handleDisconnectedB, this, _1));

	/*
	messageTimeoutA = networkFactories->getTimerFactory()->createTimer(10000);
	messageTimeoutA->onTick.connect(boost::bind(&ActiveSessionPair::handleMessageTimeoutA, this));

	messageTimeoutB = networkFactories->getTimerFactory()->createTimer(10000);
	messageTimeoutB->onTick.connect(boost::bind(&ActiveSessionPair::handleMessageTimeoutB, this));
	*/
}

ActiveSessionPair::~ActiveSessionPair() {
	delete clientB;
	delete clientA;
}

void ActiveSessionPair::start() {
	ClientOptions options;
	options.allowPLAINWithoutTLS = true;

	clientA->connect(options);
	clientB->connect(options);
}

void ActiveSessionPair::stop() {
	clientB->disconnect();
	clientA->disconnect();
}

void ActiveSessionPair::benchmark() {
	clientA->onMessageReceived.connect(boost::bind(&ActiveSessionPair::handleMessageReceivedA, this, _1));
	clientA->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
	clientB->onMessageReceived.connect(boost::bind(&ActiveSessionPair::handleMessageReceivedB, this, _1));
	clientB->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));

	begin = boost::posix_time::microsec_clock::local_time();
	sendMessageA();
	sendMessageB();
}

void ActiveSessionPair::prepareMessageTemplate() {
	messageHeaderA = "<message to=\"" + clientB->getJID().toString() + "\" type=\"chat\"><body>" + body + "</body><subject>";
	messageFooterA = "</subject></message>";

	messageHeaderB = "<message to=\"" + clientA->getJID().toString() + "\" type=\"chat\"><body>" + body + "</body><subject>";
	messageFooterB = "</subject></message>";
}

void ActiveSessionPair::sendMessageA() {
	/*
	Message::ref messageA = Message::ref(new Message());
	messageA->setBody(body);
	messageA->setTo(clientB->getJID());
	messageA->setSubject(idGenerator.generateID());
	++noOfSendMessagesA;
	sendMessagesA.push_back(MessageStamp(messageA));
	clientA->sendMessage(messageA);
	*/

	std::string id = idGenerator.generateID();
	sendMessagesA.push_back(MessageStamp(id));
	//messageTimeoutA->start();
	++noOfSendMessagesA;
	clientA->sendData(messageHeaderA);
	clientA->sendData(id);
	clientA->sendData(messageFooterA);
}

void ActiveSessionPair::sendMessageB() {
	/*
	Message::ref messageB = Message::ref(new Message());
	messageB->setBody(body);
	messageB->setTo(clientA->getJID());
	messageB->setSubject(idGenerator.generateID());
	++noOfSendMessagesB;
	sendMessagesB.push_back(MessageStamp(messageB));
	clientB->sendMessage(messageB);
	*/

	std::string id = idGenerator.generateID();
	sendMessagesB.push_back(MessageStamp(id));
	//messageTimeoutB->start();
	++noOfSendMessagesB;
	clientB->sendData(messageHeaderB);
	clientB->sendData(id);
	clientB->sendData(messageFooterB);
}

void ActiveSessionPair::handleMessageReceivedA(boost::shared_ptr<Swift::Message> msg) {
	//messageTimeoutB->stop();
	receivedMessagesA.push_back(MessageStamp(msg));
	++noOfReceivedMessagesA;
	if (noOfSendMessagesB < messages) {
		sendMessageB();
	}

	if (!done) checkIfDone();
}

void ActiveSessionPair::handleMessageReceivedB(boost::shared_ptr<Swift::Message> msg) {
	//messageTimeoutA->stop();
	receivedMessagesB.push_back(MessageStamp(msg));
	++noOfReceivedMessagesB;
	if (noOfSendMessagesA < messages) {
		sendMessageA();
	}

	if (!done) checkIfDone();
}

void ActiveSessionPair::handleMessageTimeoutA() {
	std::cout << this << " - " << clientB->getJID() << " - Message from A to B needed more than 10 seconds." << std::endl;
	std::cout << this << " - " << "Messages send to B: " << noOfSendMessagesA << std::endl;
	std::cout << this << " - " << "Messages recv'd by B: " << noOfReceivedMessagesB << std::endl;
}

void ActiveSessionPair::handleMessageTimeoutB() {
	std::cout << this << " - " << clientA->getJID() << " - Message from B to A needed more than 10 seconds." << std::endl;
	std::cout << this << " - " << "Messages send to A: " << noOfSendMessagesB << std::endl;
	std::cout << this << " - " << "Messages recv'd by A: " << noOfReceivedMessagesA << std::endl;
}

void ActiveSessionPair::handleDataRead(const SafeByteArray& data) {
	bytesReceived += data.size();
}

BenchmarkSession::LatencyInfo ActiveSessionPair::getLatencyResults() {
	BenchmarkSession::LatencyInfo info;

	std::vector<double> latencies;
	calculateLatencies(sendMessagesA, receivedMessagesB, latencies);
	calculateLatencies(sendMessagesB, receivedMessagesA, latencies);

	info.stanzas = 0;
	if (latencies.empty()) {
		return info;
	}

	std::vector<double>::iterator i = latencies.begin();
	info.stanzas++;
	info.minSeconds = info.maxSeconds = info.avgSeconds = *i;
	++i;
	for(; i != latencies.end(); ++i) {
		if (*i < info.minSeconds) {
			info.minSeconds = *i;
		} else if (*i > info.maxSeconds) {
			info.maxSeconds = *i;
		}
		info.avgSeconds += *i;
		info.stanzas++;
	}
	info.avgSeconds /= info.stanzas;

	boost::posix_time::time_duration benchmarkDuration = end - begin;
	info.bytesPerSecond = (double)bytesReceived / benchmarkDuration.seconds();
	info.stanzasPerSecond = (double)info.stanzas / benchmarkDuration.seconds();

	return info;
}

void ActiveSessionPair::calculateLatencies(std::list<MessageStamp>& sent, std::list<MessageStamp>& received, std::vector<double>& latencies) {
	std::map<std::string, boost::posix_time::ptime> receivedMap;
	for (std::list<MessageStamp>::iterator i = received.begin(); i != received.end(); ++i) {
		receivedMap[i->text] = i->timestamp;
	}

	while(!sent.empty()) {
		MessageStamp sentStamp = sent.front();
		std::map<std::string, boost::posix_time::ptime>::iterator receivedTime = receivedMap.find(sentStamp.text);
		if (receivedTime != receivedMap.end()) {
			boost::posix_time::time_duration latency = receivedTime->second - sentStamp.timestamp;
			latencies.push_back((double)latency.seconds() + ((double)latency.total_microseconds()/1000/1000));
		} else {
			std::cout << "Message with subject = " << sentStamp.text << " didn't get through!" << std::endl;
		}
		sent.pop_front();
	}
}

void ActiveSessionPair::checkIfDone() {
	if (noOfReceivedMessagesA >= messages &&
		noOfReceivedMessagesB >= messages) {
		clientA->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
		clientB->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
		end = boost::posix_time::microsec_clock::local_time();
		onDoneBenchmarking();
		done = true;
	}
}

void ActiveSessionPair::handleConnectedA() {
	++connectedClients;
	if (connectedClients == 2) {
		prepareMessageTemplate();
		onReady();
	}
}

void ActiveSessionPair::handleDisconnectedA(const boost::optional<ClientError>& error) {
	if (error) {
		std::cout << "ActiveSessionPair session disconnected with error ( " << error.get().getType() << " )!" << std::endl;
	}
}

void ActiveSessionPair::handleConnectedB() {
	++connectedClients;
	if (connectedClients == 2) {
		prepareMessageTemplate();
		onReady();
	}
}

void ActiveSessionPair::handleDisconnectedB(const boost::optional<ClientError>& error) {
	if (error) {
		std::cout << "ActiveSessionPair session disconnected with error ( " << error.get().getType() << " )!" << std::endl;
	}
}
