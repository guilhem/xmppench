/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#include "BenchmarkNetworkFactories.h"

#include <Swiften/Network/BoostConnectionFactory.h>
#include <Swiften/Network/BoostConnectionServerFactory.h>
#include <Swiften/Network/BoostTimerFactory.h>
#include <Swiften/Parser/PlatformXMLParserFactory.h>

BenchmarkNetworkFactories::BenchmarkNetworkFactories(BoostEventLoop* eventLoop, const std::string& ip) : eventLoop(eventLoop) {
	xmlParserFactory = new Swift::PlatformXMLParserFactory();
	connectionServerFactory = new Swift::BoostConnectionServerFactory(eventLoop->getIOService(), eventLoop);
	domainNameResolver = new StaticDomainNameResolver(ip);
	connectionFactory = new Swift::BoostConnectionFactory(eventLoop->getIOService(), eventLoop);
	timerFactory = new Swift::BoostTimerFactory(eventLoop->getIOService(), eventLoop);
}

BenchmarkNetworkFactories::~BenchmarkNetworkFactories() {
	delete timerFactory;
	delete connectionFactory;
	delete domainNameResolver;
	delete connectionServerFactory;
	delete xmlParserFactory;
}

Swift::TimerFactory* BenchmarkNetworkFactories::getTimerFactory() const {
	return timerFactory;
}

Swift::ConnectionFactory* BenchmarkNetworkFactories::getConnectionFactory() const {
	return connectionFactory;
}

Swift::DomainNameResolver* BenchmarkNetworkFactories::getDomainNameResolver() const {
	return domainNameResolver;
}

Swift::ConnectionServerFactory* BenchmarkNetworkFactories::getConnectionServerFactory() const {
	return connectionServerFactory;
}

Swift::NATTraverser* BenchmarkNetworkFactories::getNATTraverser() const {
	return NULL;
}

Swift::XMLParserFactory* BenchmarkNetworkFactories::getXMLParserFactory() const {
	return xmlParserFactory;
}
