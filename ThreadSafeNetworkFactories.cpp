/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#include "ThreadSafeNetworkFactories.h"

ThreadSafeNetworkFactories::ThreadSafeNetworkFactories(Swift::EventLoop* eventLoop, const std::string& ip) : domainNameResolver(ip), eventLoop(eventLoop), networkFactories(eventLoop) {
	//domainNameResolver = new StaticDomainNameResolver(ip);
}

ThreadSafeNetworkFactories::~ThreadSafeNetworkFactories() {
	//delete domainNameResolver;
}

Swift::TimerFactory* ThreadSafeNetworkFactories::getTimerFactory() const {
	return networkFactories.getTimerFactory();
}

Swift::ConnectionFactory* ThreadSafeNetworkFactories::getConnectionFactory() const {
	return networkFactories.getConnectionFactory();
}

Swift::DomainNameResolver* ThreadSafeNetworkFactories::getDomainNameResolver() const {
	return const_cast<StaticDomainNameResolver*>(&domainNameResolver);
}

Swift::ConnectionServerFactory* ThreadSafeNetworkFactories::getConnectionServerFactory() const {
	return networkFactories.getConnectionServerFactory();
}
