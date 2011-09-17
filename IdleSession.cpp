#include "IdleSession.h"

#include <boost/bind.hpp>

using namespace Swift;

IdleSession::IdleSession(AccountDataProvider* accountDataProvider, Swift::NetworkFactories* networkFactories, Swift::CertificateTrustChecker* trustChecker) {
	AccountDataProvider::Account account = accountDataProvider->getAccount();

	client = new CoreClient(Swift::JID(account.jid), createSafeByteArray(account.password), networkFactories);
	client->setCertificateTrustChecker(trustChecker);
	client->onConnected.connect(boost::bind(&IdleSession::handleConnected, this));
	client->onDisconnected.connect(boost::bind(&IdleSession::handleDisconnected, this, _1));
}

IdleSession::~IdleSession() {

}

void IdleSession::start() {
	ClientOptions options;
	options.allowPLAINWithoutTLS = true;

	client->connect(options);
}

void IdleSession::stop() {
	client->disconnect();
}

void IdleSession::benchmark() {
	// no op; we're idle here
	onDoneBenchmarking();
}

void IdleSession::handleConnected() {
	onReady();
}

void IdleSession::handleDisconnected(const boost::optional<ClientError>& error) {
	if (error) {
		std::cout << "Idle session disconnected with error ( " << error.get().getType() << " )!" << std::endl;
	}
}
