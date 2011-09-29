/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include <Swiften/EventLoop/EventLoop.h>

class BoostEventLoop : public Swift::EventLoop {
public:
	BoostEventLoop();
	void run();
	virtual void post(const Swift::Event& event);

	boost::shared_ptr<boost::asio::io_service> getIOService() const;

private:
	boost::shared_ptr<boost::asio::io_service> ioService_;
};
