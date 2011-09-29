#include "BoostEventLoop.h"

#include <boost/asio.hpp>
#include <boost/smart_ptr/make_shared.hpp>

BoostEventLoop::BoostEventLoop() {
	ioService_ = boost::make_shared<boost::asio::io_service>();
}

void BoostEventLoop::run() {
	ioService_->run();
}

void BoostEventLoop::post(const Swift::Event& event) {
	event.callback();
}

boost::shared_ptr<boost::asio::io_service> BoostEventLoop::getIOService() const {
	return ioService_;
}
