/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

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
