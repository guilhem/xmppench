/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#pragma once

#include <string>

#include <Swiften/Network/DomainNameAddressQuery.h>
#include <Swiften/Network/DomainNameResolver.h>
#include <Swiften/Network/DomainNameServiceQuery.h>

class StaticDomainNameResolver : public Swift::DomainNameResolver {
public:
	StaticDomainNameResolver(const std::string&);
	virtual ~StaticDomainNameResolver();

	virtual boost::shared_ptr<Swift::DomainNameServiceQuery> createServiceQuery(const std::string& ip);
	virtual boost::shared_ptr<Swift::DomainNameAddressQuery> createAddressQuery(const std::string& ip);

private:
	class StaticDomainNameAddressQuery : public Swift::DomainNameAddressQuery {
	public:
		StaticDomainNameAddressQuery(const std::string& ip);
		virtual ~StaticDomainNameAddressQuery();
		virtual void run();

	private:
		std::string ip;
	};

	class StaticDomainNameServiceQuery : public Swift::DomainNameServiceQuery {
	public:
		StaticDomainNameServiceQuery(const std::string& ip);
		virtual ~StaticDomainNameServiceQuery();
		virtual void run();

	private:
		std::string ip;
	};

private:
	std::string ip;
};
