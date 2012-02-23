/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#pragma once

#include <vector>

#include <boost/signal.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

class BenchmarkSession {
public:
	struct LatencyInfo {
		double minSeconds;
		double maxSeconds;
		double avgSeconds;
		double sdSeconds;

		std::vector<double> latencies;

		unsigned long stanzas;

		double receivedBytes;
		double bytesPerSecond;
		double stanzasPerSecond;

		// helpers
		double sum;
		double sumOfSquared;

	};

public:
	virtual ~BenchmarkSession() {}

	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void benchmark(const boost::posix_time::ptime& now) = 0;

	virtual LatencyInfo getLatencyResults() { return LatencyInfo(); }

	boost::signal<void ()> onReadyToBenchmark;
	boost::signal<void ()> onDoneBenchmarking;
	boost::signal<void ()> onBenchmarkEnd;
	boost::signal<void ()> onStopped;
};
