/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#pragma once

#include <Swiften/Client/CoreClient.h>
#include <Swiften/Network/NetworkFactories.h>
#include <Swiften/TLS/CertificateTrustChecker.h>
#include <Swiften/Base/URL.h>

#include "AccountDataProvider.h"
#include "BenchmarkSession.h"

class IdleSession : public BenchmarkSession {
public:
	IdleSession(AccountDataProvider* accountDataProvider, Swift::NetworkFactories* networkFactories, Swift::CertificateTrustChecker* trustChecker, bool noCompression, bool noTLS, const Swift::URL& boshURL);
	virtual ~IdleSession();

	virtual void start();
	virtual void stop();
	virtual void benchmark(const boost::posix_time::ptime& now);

private:
	void handleConnected();
	void handleDisconnected(const boost::optional<Swift::ClientError>&);

private:
	Swift::CoreClient* client;
	bool noCompression;
	bool noTLS;
	Swift::URL boshURL;
};
