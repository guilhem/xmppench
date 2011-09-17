/*
  Copyright (c) 2007-2008 by Tobias Markmann <tm@ayena.de>

  This software is distributed without any warranty.
*/

#include <sys/time.h>

//#include <ctime>
#include <string>

class Time {
	long m_microsecond;
	int m_second;
	int m_minute;
	int m_hour;
	int m_day;
	int m_month;
	int m_year;
public:
	Time();
	~Time();
	
	// operators
	Time& operator=(const Time &rhs);
	Time& operator+=(const Time &rhs);
	Time& operator-=(const Time &rhs);
	Time& operator+(const Time &rhs);
	Time& operator-(const Time &rhs);
	
	void now();
	
	long microsecond() const;
	int second() const;
	double seconds() const;
	int minute() const;
	int hour() const;
	int day() const;
	int month() const;
	int year() const;
};
