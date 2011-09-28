/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#include "StaticDomainNameResolver.h"

#include <vector>

#include <boost/smart_ptr/make_shared.hpp>

#include <Swiften/Network/HostAddress.h>
#include <Swiften/Network/HostAddressPort.h>


StaticDomainNameResolver::StaticDomainNameResolver(const std::string& ip) : ip(ip) { }
StaticDomainNameResolver::~StaticDomainNameResolver() { }

boost::shared_ptr<Swift::DomainNameServiceQuery> StaticDomainNameResolver::createServiceQuery(const std::string&) {
	return boost::make_shared<StaticDomainNameServiceQuery>(ip);
}

boost::shared_ptr<Swift::DomainNameAddressQuery> StaticDomainNameResolver::createAddressQuery(const std::string&) {
	return boost::make_shared<StaticDomainNameAddressQuery>(ip);
}

StaticDomainNameResolver::StaticDomainNameAddressQuery::StaticDomainNameAddressQuery(const std::string& ip) : ip(ip) { }
StaticDomainNameResolver::StaticDomainNameAddressQuery::~StaticDomainNameAddressQuery() { }

void StaticDomainNameResolver::StaticDomainNameAddressQuery::run() {
	std::vector<Swift::HostAddress> addresses;
	addresses.push_back(Swift::HostAddress(ip));
	onResult(addresses, boost::optional<Swift::DomainNameResolveError>());
}

StaticDomainNameResolver::StaticDomainNameServiceQuery::StaticDomainNameServiceQuery(const std::string& ip) : ip(ip) { }
StaticDomainNameResolver::StaticDomainNameServiceQuery::~StaticDomainNameServiceQuery() { }

void StaticDomainNameResolver::StaticDomainNameServiceQuery::run() {
	std::vector<Swift::DomainNameServiceQuery::Result> results;
	//results.push_back(Swift::DomainNameServiceQuery::Result());
	onResult(results);
}
