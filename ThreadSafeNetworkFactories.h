/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#pragma once

#include <Swiften/EventLoop/EventLoop.h>
#include <Swiften/Network/BoostNetworkFactories.h>
#include <Swiften/Network/NetworkFactories.h>

#include "StaticDomainNameResolver.h"

class ThreadSafeNetworkFactories : public Swift::NetworkFactories {
public:
	ThreadSafeNetworkFactories(Swift::EventLoop*, const std::string&);
	virtual ~ThreadSafeNetworkFactories();

	virtual Swift::TimerFactory* getTimerFactory() const;
	virtual Swift::ConnectionFactory* getConnectionFactory() const;
	virtual Swift::DomainNameResolver* getDomainNameResolver() const;
	virtual Swift::ConnectionServerFactory* getConnectionServerFactory() const;

private:
	StaticDomainNameResolver domainNameResolver;
	Swift::EventLoop* eventLoop;
	Swift::BoostNetworkFactories networkFactories;
};
