//
// Boost의 date 객체를 이용하면 날짜 계산을 쉽게 할 수 있다.
//
#include "stdafx.h"
#include "date.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace common;

// 2017-01-08-11-05-010
// year-month-day-hour-minutes-seconds-millseconds
// yyyy-mm-dd-hh-mm-ss-mmm
string common::GetCurrentDateTime()
{
	// http://stackoverflow.com/questions/22975077/how-to-convert-a-boostptime-to-string

	using namespace boost::gregorian;
	using namespace boost::posix_time;

	boost::gregorian::date dayte(boost::gregorian::day_clock::local_day());
	boost::posix_time::ptime midnight(dayte);
	boost::posix_time::ptime
		now(boost::posix_time::microsec_clock::local_time());
	boost::posix_time::time_duration td = now - midnight;

	date::ymd_type ymd = second_clock::local_time().date().year_month_day();

	stringstream ss;
	try {
		ss << ymd.year 
			<< "-" << std::setfill('0') << std::setw(2) << ymd.month.as_number() 
			<< "-" << std::setfill('0') << std::setw(2) << ymd.day
			<< "-" << std::setfill('0') << std::setw(2) << td.hours() 
			<< "-" << std::setfill('0') << std::setw(2) << td.minutes() 
			<< "-" << std::setfill('0') << std::setw(2) << td.seconds() 
			<< "-" << std::setfill('0') << std::setw(3) << td.total_milliseconds()%1000;
	}
	catch (...)
	{
		// nothing~
	}

	return ss.str();
}


// 201701081105010
// year-month-day-hour-minutes-seconds-millseconds
// yyyymmddhhmmssmmm
string common::GetCurrentDateTime2()
{
	// http://stackoverflow.com/questions/22975077/how-to-convert-a-boostptime-to-string

	using namespace boost::gregorian;
	using namespace boost::posix_time;

	boost::gregorian::date dayte(boost::gregorian::day_clock::local_day());
	boost::posix_time::ptime midnight(dayte);
	boost::posix_time::ptime
		now(boost::posix_time::microsec_clock::local_time());
	boost::posix_time::time_duration td = now - midnight;

	date::ymd_type ymd = second_clock::local_time().date().year_month_day();

	stringstream ss;
	try {
		ss << ymd.year
			<< std::setfill('0') << std::setw(2) << ymd.month.as_number()
			<< std::setfill('0') << std::setw(2) << ymd.day
			<< std::setfill('0') << std::setw(2) << td.hours()
			<< std::setfill('0') << std::setw(2) << td.minutes()
			<< std::setfill('0') << std::setw(2) << td.seconds()
			<< std::setfill('0') << std::setw(3) << td.total_milliseconds() % 1000;
	}
	catch (...)
	{
		// nothing~
	}

	return ss.str();
}


// 201701081105010
// year-month-day-hour-minutes-seconds-millseconds
// yyyymmddhhmmssmmm
unsigned __int64 common::GetCurrentDateTime3()
{
	// http://stackoverflow.com/questions/22975077/how-to-convert-a-boostptime-to-string

	using namespace boost::gregorian;
	using namespace boost::posix_time;

	boost::gregorian::date dayte(boost::gregorian::day_clock::local_day());
	boost::posix_time::ptime midnight(dayte);
	boost::posix_time::ptime
		now(boost::posix_time::microsec_clock::local_time());
	boost::posix_time::time_duration td = now - midnight;

	date::ymd_type ymd = second_clock::local_time().date().year_month_day();

	unsigned __int64 ret = 0;
	try {
		ret = (unsigned __int64)ymd.year * 10000000000000
		+ (unsigned __int64)ymd.month.as_number() * 100000000000
		+ (unsigned __int64)ymd.day * 1000000000
		+ (unsigned __int64)td.hours() * 10000000
		+ (unsigned __int64)td.minutes() * 100000
		+ (unsigned __int64)td.seconds() * 1000
		+ (unsigned __int64)td.total_milliseconds() % 1000;
	}
	catch (...)
	{
		// nothing~
	}

	return ret;
}


//--------------------------------------------------------------------------------------
cDateTime::cDateTime()
{
}

cDateTime::cDateTime(const string &strDate)
{

}

cDateTime::~cDateTime()
{
}


bool cDateTime::Parse(const char *str)
{
	return true;
}
