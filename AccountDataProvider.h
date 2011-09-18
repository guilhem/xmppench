#pragma once

#include <Swiften/JID/JID.h>

class AccountDataProvider {
public:
	struct Account {
		Swift::JID jid;
		std::string password;
	};

public:
	virtual Account getAccount() = 0;
};