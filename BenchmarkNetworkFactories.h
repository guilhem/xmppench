/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#pragma once

#include <Swiften/EventLoop/EventLoop.h>
#include <Swiften/Network/NetworkFactories.h>


#include "BoostEventLoop.h"
#include "StaticDomainNameResolver.h"

class BenchmarkNetworkFactories : public Swift::NetworkFactories {
public:
	BenchmarkNetworkFactories(BoostEventLoop*, const std::string&);
	virtual ~BenchmarkNetworkFactories();

	virtual Swift::TimerFactory* getTimerFactory() const;
	virtual Swift::ConnectionFactory* getConnectionFactory() const;
	virtual Swift::DomainNameResolver* getDomainNameResolver() const;
	virtual Swift::ConnectionServerFactory* getConnectionServerFactory() const;
	virtual Swift::NATTraverser* getNATTraverser() const;
	virtual Swift::XMLParserFactory* getXMLParserFactory() const;

private:
	Swift::TimerFactory* timerFactory;
	Swift::ConnectionFactory* connectionFactory;
	StaticDomainNameResolver* domainNameResolver;
	Swift::ConnectionServerFactory* connectionServerFactory;
	Swift::XMLParserFactory* xmlParserFactory;

	BoostEventLoop *eventLoop;
};
