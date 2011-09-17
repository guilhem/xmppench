/*
  Copyright (c) 2007-2009 by Tobias Markmann <tm@ayena.de>

  This software is distributed without any warranty.
*/

#include "DateTime.h"

Time::Time() {
	now();
}

Time::~Time() {

}

Time& Time::operator=(const Time &rhs) {
	if (this == &rhs) return *this; 
	m_microsecond = rhs.m_microsecond;
	m_second = rhs.m_second;
	m_minute = rhs.m_minute;
	m_hour = rhs.m_hour;
	m_day = rhs.m_day;
	m_month = rhs.m_month;
	m_year = rhs.m_year;
	return *this;
}

Time& Time::operator+=(const Time &rhs) {
	time_t this_time, rhs_time;
	tm tm_t;
	tm *tm_s;
	tm_t.tm_sec = m_second;
	tm_t.tm_min = m_minute;
	tm_t.tm_hour = m_hour;
	tm_t.tm_mday = m_day;
	tm_t.tm_mon = m_month - 1;
	tm_t.tm_year = m_year - 1900;
	this_time = mktime(&tm_t);
	
	tm_t.tm_sec = rhs.m_second;
	tm_t.tm_min = rhs.m_minute;
	tm_t.tm_hour = rhs.m_hour;
	tm_t.tm_mday = rhs.m_day;
	tm_t.tm_mon = rhs.m_month - 1;
	tm_t.tm_year = rhs.m_year - 1900;
	rhs_time = mktime(&tm_t);

	if ((m_microsecond + rhs.m_microsecond) > 999999) {
		m_microsecond -= 1000000;
		this_time += 1;
	}
	m_microsecond += rhs.m_microsecond;
	this_time += rhs_time;
	
	tm_s = localtime(&(this_time));
	m_second = tm_s->tm_sec;
	m_minute = tm_s->tm_min;
	m_hour = tm_s->tm_hour;
	m_day = tm_s->tm_mday;
	m_month = tm_s->tm_mon + 1;
	m_year = tm_s->tm_year + 1900;
	return *this;
}

Time& Time::operator-=(const Time &rhs) {
	time_t this_time, rhs_time;
	tm tm_t;
	tm *tm_s;
	tm_t.tm_sec = m_second;
	tm_t.tm_min = m_minute;
	tm_t.tm_hour = m_hour;
	tm_t.tm_mday = m_day;
	tm_t.tm_mon = m_month - 1;
	tm_t.tm_year = m_year - 1900;
	this_time = mktime(&tm_t);
	
	tm_t.tm_sec = rhs.m_second;
	tm_t.tm_min = rhs.m_minute;
	tm_t.tm_hour = rhs.m_hour;
	tm_t.tm_mday = rhs.m_day;
	tm_t.tm_mon = rhs.m_month - 1;
	tm_t.tm_year = rhs.m_year - 1900;
	rhs_time = mktime(&tm_t);

	if ((m_microsecond - rhs.m_microsecond) < 0) {
		m_microsecond += 1000000;
		this_time -= 1;
	}
	m_microsecond -= rhs.m_microsecond;
	this_time -= rhs_time;
	
	tm_s = localtime(&(this_time));
	m_second = tm_s->tm_sec;
	m_minute = tm_s->tm_min;
	m_hour = tm_s->tm_hour;
	m_day = tm_s->tm_mday;
	m_month = tm_s->tm_mon + 1;
	m_year = tm_s->tm_year + 1900;
	return *this;
}

Time& Time::operator+(const Time &rhs) {
	return Time(*this) += rhs;
}

Time& Time::operator-(const Time &rhs) {
	return Time(*this) -= rhs;
}

void Time::now() {
	timeval now;
	tm *tm_s;
	gettimeofday(&now, NULL);
	tm_s = localtime(&(now.tv_sec));
	m_microsecond = now.tv_usec;
	m_second = tm_s->tm_sec;
	m_minute = tm_s->tm_min;
	m_hour = tm_s->tm_hour;
	m_day = tm_s->tm_mday;
	m_month = tm_s->tm_mon + 1;
	m_year = tm_s->tm_year + 1900;
}
	
long Time::microsecond() const {
	return m_microsecond;
}

int Time::second() const {
	return m_second;
}

double Time::seconds() const {
	double result = second();
	result += (double) microsecond() / 1000 / 1000 / 1000;
	return result;
}

int Time::minute() const {
	return m_minute;
}

int Time::hour() const {
	return m_hour;
}

int Time::day() const {
	return m_day;
}

int Time::month() const {
	return m_month;
}

int Time::year() const {
	return m_year;
}

