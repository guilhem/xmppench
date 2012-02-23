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

ActiveSessionPair::ActiveSessionPair(AccountDataProvider* accountDataProvider, Swift::NetworkFactories* networkFactories, Swift::CertificateTrustChecker* trustChecker, int warmUpMessages, int messages, std::string body, bool noCompression, bool noTLS, const Swift::URL& boshURL) :
	accountDataProvider(accountDataProvider), networkFactories(networkFactories), trustChecker(trustChecker), warmUpMessages(warmUpMessages), messages(messages), body(body), noCompression(noCompression), noTLS(noTLS), connectedClients(0), bytesReceived(0), benchmarking(false), boshURL(boshURL) {
	noOfSentMessages[0] = 0;
	noOfSentMessages[1] = 0;
	noOfReceivedMessages[0] = 0;
	noOfReceivedMessages[1] = 0;
	benchmarkingDone = false;
	dataCounting[0] = dataCounting[1] = false;

	for (int i = 0; i < 2; i++) {
		AccountDataProvider::Account acc = accountDataProvider->getAccount();
		client[i] = new CoreClient(Swift::JID(acc.jid), createSafeByteArray(acc.password), networkFactories);
		client[i]->setCertificateTrustChecker(trustChecker);
		client[i]->onConnected.connect(boost::bind(&ActiveSessionPair::handleConnected, this, i));
		client[i]->onDisconnected.connect(boost::bind(&ActiveSessionPair::handleDisconnected, this, i, _1));
	}

	/*
	messageTimeoutA = networkFactories->getTimerFactory()->createTimer(10000);
	messageTimeoutA->onTick.connect(boost::bind(&ActiveSessionPair::handleMessageTimeoutA, this));

	messageTimeoutB = networkFactories->getTimerFactory()->createTimer(10000);
	messageTimeoutB->onTick.connect(boost::bind(&ActiveSessionPair::handleMessageTimeoutB, this));
	*/
}

ActiveSessionPair::~ActiveSessionPair() {
	delete client[1];
	delete client[0];
}

void ActiveSessionPair::start() {
	ClientOptions options;
	options.allowPLAINWithoutTLS = true;
	options.useStreamCompression = !noCompression;
	options.useAcks = false;
	options.useTLS = noTLS ? ClientOptions::NeverUseTLS : ClientOptions::UseTLSWhenAvailable;
	options.boshURL = boshURL;

	client[0]->connect(options);
	client[1]->connect(options);
}

void ActiveSessionPair::stop() {
	done = true;
	client[0]->onDataRead.disconnect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
	client[1]->onDataRead.disconnect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
	end = boost::posix_time::microsec_clock::local_time();
	client[1]->disconnect();
	client[0]->disconnect();
	onDoneBenchmarking();
}

void ActiveSessionPair::warmUp() {
	for (int i = 0; i < 2; i++) {
		client[i]->onMessageReceived.connect(boost::bind(&ActiveSessionPair::handleMessageReceived, this, i, _1));
	}
	if (warmUpMessages == 0) {
		onReadyToBenchmark();
	}
	else {
		sendMessage(0);
		sendMessage(1);
	}
}

void ActiveSessionPair::benchmark(const boost::posix_time::ptime& now) {
	begin = now;
	benchmarking = true;
	for (int i = 0; i < 2; i++) {
		client[i]->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
	}

	noOfSentMessages[0] = 0;
	noOfSentMessages[1] = 0;
	if (warmUpMessages == 0) {
		sendMessage(0);
		sendMessage(1);
	}
}

void ActiveSessionPair::prepareMessageTemplate() {
	for (int i = 0; i < 2; i++) {
		messageHeader[i] = "<message to=\"" + client[1 - i]->getJID().toString() + "\" type=\"chat\"><body>" + body + "</body><subject>";
		messageFooter[i] = "</subject></message>";
	}
}

void ActiveSessionPair::sendMessage(int connection) {
	std::string id = idGenerator.generateID();
	if (benchmarking && noOfSentMessages[connection] < messages) {
		sentMessages[connection].push_back(MessageStamp(id));
	}
	//messageTimeout[connection]->start();
	++noOfSentMessages[connection];
	client[connection]->sendData(messageHeader[connection] + id + messageFooter[connection]);
}

void ActiveSessionPair::handleMessageReceived(int connection, boost::shared_ptr<Swift::Message> msg) {
	receivedMessages[connection].push_back((MessageStamp(msg)));
	++noOfReceivedMessages[connection];
	if (noOfReceivedMessages[connection] == warmUpMessages && noOfReceivedMessages[1 - connection] >= warmUpMessages) {
		onReadyToBenchmark();
	}
	else if (noOfReceivedMessages[connection] == messages) {
		client[connection]->onDataRead.disconnect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
	}
	if (!done) {
		sendMessage(1 - connection);
	}

	if (!benchmarkingDone) checkIfDoneBenchmarking();
}

void ActiveSessionPair::handleMessageTimeout(int connection) {
	std::string me(connection == 0 ? "A" : "B");
	std::string them(connection == 1 ? "A" : "B");
	std::cout << this << " - " << client[1 - connection]->getJID() << " - Message from " << me << " to " << them << " needed more than 10 seconds." << std::endl;
	std::cout << this << " - " << "Messages send to " << them << ": " << noOfSentMessages[connection] << std::endl;
	std::cout << this << " - " << "Messages recv'd by " << them << " : " << noOfReceivedMessages[1 - connection] << std::endl;
}

void ActiveSessionPair::handleDataRead(const SafeByteArray& data) {
	bytesReceived += data.size();
}

BenchmarkSession::LatencyInfo ActiveSessionPair::getLatencyResults() {
	BenchmarkSession::LatencyInfo info;

	std::vector<double> latencies;
	calculateLatencies(sentMessages[0], receivedMessages[1], latencies);
	calculateLatencies(sentMessages[1], receivedMessages[0], latencies);
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
	info.bytesPerSecond = static_cast<double>(bytesReceived) / benchmarkDuration.seconds();
	info.stanzasPerSecond = static_cast<double>(info.stanzas) / benchmarkDuration.seconds();
	info.receivedBytes = static_cast<double>(bytesReceived);
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
			latencies.push_back((static_cast<double>(latency.total_microseconds())/1000/1000));
		} else {
			std::cout << "Message with subject = " << sentStamp.text << " didn't get through!" << std::endl;
		}
		sent.pop_front();
	}
}

void ActiveSessionPair::checkIfDoneBenchmarking() {
	if (!benchmarkingDone && noOfReceivedMessages[0] >= messages &&
		noOfReceivedMessages[1] >= messages) {
		benchmarkingDone = true;
		onBenchmarkEnd();
	}
}

void ActiveSessionPair::handleConnected(int /*connection*/) {
	++connectedClients;
	if (connectedClients == 2) {
		prepareMessageTemplate();
		if (warmUpMessages == 0) {
			client[0]->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
			client[1]->onDataRead.connect(boost::bind(&ActiveSessionPair::handleDataRead, this, _1));
		}
		onReadyToWarmUp();
	}
}

void ActiveSessionPair::handleDisconnected(int connection, const boost::optional<ClientError>& error) {
	if (error) {
		std::cout << "ActiveSessionPair session disconnected with error ( " << error.get().getType() << " )!" << std::endl;
	}
	if (!client[1 - connection]->isActive()) {
		onStopped();
	}
}


