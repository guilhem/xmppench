/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the GNU General Public License v3.
 * See gpl-3.0.txt for more information.
 */

#pragma once

#include <vector>

#include <boost/signal.hpp>

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
	virtual void benchmark() = 0;

	virtual LatencyInfo getLatencyResults() { return LatencyInfo(); }

	boost::signal<void ()> onReady;
	boost::signal<void ()> onDoneBenchmarking;
	boost::signal<void ()> onBenchmarkStart;
	boost::signal<void ()> onBenchmarkEnd;
	boost::signal<void ()> onStopped;
};
