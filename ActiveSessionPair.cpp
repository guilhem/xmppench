/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#include "ActiveSessionPair.h"

#include <algorithm>

#include <Swiften/Base/SafeByteArray.h>
#include <Swiften/Elements/Message.h>
#include <Swiften/Network/NetworkFactories.h>
#include <Swiften/Network/TimerFactory.h>
#include <Swiften/TLS/CertificateTrustChecker.h>

using namespace Swift;

ActiveSessionPair::ActiveSessionPair(AccountDataProvider* accountDataProvider, Swift::NetworkFactories* networkFactories, Swift::CertificateTrustChecker* trustChecker, int warmUpMessages, int messages, std::string body, bool noCompression, bool noTLS) :
	accountDataProvider(accountDataProvider), networkFactories(networkFactories), trustChecker(trustChecker), warmUpMessages(warmUpMessages), messages(messages), body(body), noCompression(noCompression), noTLS(noTLS), connectedClients(0), noOfSendMessagesFromAToB(0), noOfReceivedMessagesByAFromB(0), noOfSendMessagesFromBToA(0), noOfReceivedMessagesByBFromA(0), bytesReceived(0) {
	benchmarkingStartedA = benchmarkingDone = benchmarkingStartedB = false;
	dataCountingForA = dataCountingForB = false;
	totalMessages = messages + 2 * warmUpMessages;
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
	options.useStreamCompression = !noCompression;
	options.useAcks = false;
	options.useTLS = noTLS ? ClientOptions::NeverUseTLS : ClientOptions::UseTLSWhenAvailable;

	clientA->connect(options);
	clientB->connect(options);
}

void ActiveSessionPair::stop() {
	clientB->disconnect();
	clientA->disconnect();
}

void ActiveSessionPair::benchmark() {
	clientA->onMessageReceived.connect(boost::bind(&ActiveSessionPair::handleMessageReceivedByAFromB, this, _1));
	clientB->onMessageReceived.connect(boost::bind(&ActiveSessionPair::handleMessageReceivedByBFromA, this, _1));

	sendMessageFromAToB();
	sendMessageFromBToA();
}

void ActiveSessionPair::prepareMessageTemplate() {
	messageHeaderA = "<message to=\"" + clientB->getJID().toString() + "\" type=\"chat\"><body>" + body + "</body><subject>";
	messageFooterA = "</subject></message>";

	messageHeaderB = "<message to=\"" + clientA->getJID().toString() + "\" type=\"chat\"><body>" + body + "</body><subject>";
	messageFooterB = "</subject></message>";
}

void ActiveSessionPair::sendMessageFromAToB() {
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
	if (noOfSendMessagesFromAToB >= warmUpMessages && noOfSendMessagesFromAToB < (totalMessages - warmUpMessages)) {
		if (!benchmarkingStartedA) {
			benchmarkingStartedA = true;
			if (!benchmarkingStartedB) onBenchmarkStart();
		}
		sendMessagesFromA.push_back(MessageStamp(id));
		//std::cout << "sendMessageFromAToB: benchmarked " << noOfSendMessagesFromAToB<< std::endl;
	}
	//messageTimeoutA->start();
	//std::cout << "sendMessageFromAToB: no = " << noOfSendMessagesFromAToB << " benchmarkingStartedA = " << benchmarkingStartedA << std::endl;
	++noOfSendMessagesFromAToB;
	clientA->sendData(messageHeaderA + id + messageFooterA);
}

void ActiveSessionPair::sendMessageFromBToA() {
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
	if (noOfSendMessagesFromBToA >= warmUpMessages && noOfSendMessagesFromBToA < (totalMessages - warmUpMessages)) {
		if (!benchmarkingStartedB) {
			benchmarkingStartedB = true;
			if (!benchmarkingStartedB) onBenchmarkStart();
			//clientB->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
		}
		sendMessagesFromB.push_back(MessageStamp(id));
		//std::cout << "sendMessageFromBToA: benchmarked " << noOfSendMessagesFromBToA << std::endl;
	} else if (noOfReceivedMessagesByBFromA == (totalMessages - warmUpMessages)) {
		//clientB->onDataRead.disconnect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
	}
	//messageTimeoutB->start();
	//std::cout << "sendMessageFromBToA: no = " << noOfSendMessagesFromBToA << " benchmarkingStartedB = " << benchmarkingStartedB << std::endl;
	++noOfSendMessagesFromBToA;
	clientB->sendData(messageHeaderB + id + messageFooterB);
}

void ActiveSessionPair::handleMessageReceivedByAFromB(boost::shared_ptr<Swift::Message> msg) {
	receivedMessagesA.push_back((MessageStamp(msg)));
	++noOfReceivedMessagesByAFromB;
	if (noOfReceivedMessagesByAFromB == warmUpMessages) {
		clientA->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
	}
	if (noOfReceivedMessagesByAFromB == messages + warmUpMessages) {
		clientA->onDataRead.disconnect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
	}

	if (noOfSendMessagesFromBToA < totalMessages) {
		sendMessageFromBToA();
	}

	if (!benchmarkingDone) checkIfDoneBenchmarking();
	if (!done) checkIfDone();
}
void ActiveSessionPair::handleMessageReceivedByBFromA(boost::shared_ptr<Swift::Message> msg) {
	//messageTimeoutA->stop();
	receivedMessagesB.push_back(MessageStamp(msg));
	++noOfReceivedMessagesByBFromA;
	if (noOfReceivedMessagesByBFromA == warmUpMessages) {
		clientB->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
	}
	if (noOfReceivedMessagesByBFromA == messages + warmUpMessages) {
		clientB->onDataRead.disconnect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
	}

	if (noOfSendMessagesFromAToB < totalMessages) {
		sendMessageFromAToB();
	}

	if (!benchmarkingDone) checkIfDoneBenchmarking();
	if (!done) checkIfDone();
}

void ActiveSessionPair::handleMessageTimeoutA() {
	std::cout << this << " - " << clientB->getJID() << " - Message from A to B needed more than 10 seconds." << std::endl;
	std::cout << this << " - " << "Messages send to B: " << noOfSendMessagesFromAToB << std::endl;
	std::cout << this << " - " << "Messages recv'd by B: " << noOfReceivedMessagesByBFromA << std::endl;
}

void ActiveSessionPair::handleMessageTimeoutB() {
	std::cout << this << " - " << clientA->getJID() << " - Message from B to A needed more than 10 seconds." << std::endl;
	std::cout << this << " - " << "Messages send to A: " << noOfSendMessagesFromBToA << std::endl;
	std::cout << this << " - " << "Messages recv'd by A: " << noOfReceivedMessagesByAFromB << std::endl;
}

void ActiveSessionPair::handleDataRead(const SafeByteArray& data) {
	bytesReceived += data.size();
}

BenchmarkSession::LatencyInfo ActiveSessionPair::getLatencyResults() {
	BenchmarkSession::LatencyInfo info;

	std::vector<double> latencies;
	calculateLatencies(sendMessagesFromA, receivedMessagesB, latencies);
	calculateLatencies(sendMessagesFromB, receivedMessagesA, latencies);
	info.latencies = latencies;

	info.stanzas = 0;
	if (latencies.empty()) {
		return info;
	}

	std::vector<double>::iterator i = latencies.begin();
	info.stanzas++;
	info.minSeconds = info.maxSeconds = info.avgSeconds = *i;

	// helper code
	info.sum = *i;
	info.sumOfSquared = (*i)*(*i);

	++i;
	for(; i != latencies.end(); ++i) {
		if (*i < info.minSeconds) {
			info.minSeconds = *i;
		} else if (*i > info.maxSeconds) {
			info.maxSeconds = *i;
		}
		info.avgSeconds += *i;
		info.stanzas++;

		// helper code
		info.sum += *i;
		info.sumOfSquared += (*i) * (*i);
	}
	info.avgSeconds /= info.stanzas;

	boost::posix_time::time_duration benchmarkDuration = end - begin;
	info.bytesPerSecond = (double)bytesReceived / benchmarkDuration.seconds();
	info.stanzasPerSecond = (double)info.stanzas / benchmarkDuration.seconds();
	info.receivedBytes = (double)bytesReceived;
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
			latencies.push_back(((double)latency.total_microseconds()/1000/1000));
		} else {
			std::cout << "Message with subject = " << sentStamp.text << " didn't get through!" << std::endl;
		}
		sent.pop_front();
	}
}

void ActiveSessionPair::checkIfDoneBenchmarking() {
	if (!benchmarkingDone && noOfReceivedMessagesByAFromB >= (totalMessages - warmUpMessages) &&
		noOfReceivedMessagesByBFromA >= (totalMessages - warmUpMessages)) {
		benchmarkingDone = true;
		onBenchmarkEnd();
	}
}

void ActiveSessionPair::checkIfDone() {
	if (noOfReceivedMessagesByAFromB >= totalMessages &&
		noOfReceivedMessagesByBFromA >= totalMessages) {
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
		if (warmUpMessages == 0) {
			clientA->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
			clientB->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
		}
		onReady();
	}
}

void ActiveSessionPair::handleDisconnectedA(const boost::optional<ClientError>& error) {
	if (error) {
		std::cout << "ActiveSessionPair session disconnected with error ( " << error.get().getType() << " )!" << std::endl;
	}
	if (!clientB->isActive()) {
		onStopped();
	}
}

void ActiveSessionPair::handleConnectedB() {
	++connectedClients;
	if (connectedClients == 2) {
		prepareMessageTemplate();
		if (warmUpMessages == 0) {
			clientA->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
			clientB->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
		}
		onReady();
	}
}

void ActiveSessionPair::handleDisconnectedB(const boost::optional<ClientError>& error) {
	if (error) {
		std::cout << "ActiveSessionPair session disconnected with error ( " << error.get().getType() << " )!" << std::endl;
	}
	if (!clientA->isActive()) {
		onStopped();
	}
}
