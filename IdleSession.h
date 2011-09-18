#pragma once

#include <Swiften/Client/CoreClient.h>
#include <Swiften/Network/NetworkFactories.h>
#include <Swiften/TLS/CertificateTrustChecker.h>

#include "AccountDataProvider.h"
#include "BenchmarkSession.h"

class IdleSession : public BenchmarkSession {
public:
	IdleSession(AccountDataProvider* accountDataProvider, Swift::NetworkFactories* networkFactories, Swift::CertificateTrustChecker* trustChecker);
	virtual ~IdleSession();

	virtual void start();
	virtual void stop();
	virtual void benchmark();

private:
	void handleConnected();
	void handleDisconnected(const boost::optional<Swift::ClientError>&);

private:
	Swift::CoreClient* client;
};
