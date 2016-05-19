// __!!rengine_copyright!!__ //

#include <rengine/lang/Lang.h>
#include <string>

#ifndef __RENGINE_TIMESTAMP_H__
#define __RENGINE_TIMESTAMP_H__

namespace rengine
{
	class Timestamp
	{
	public:
		enum Month
		{
			January		=  1,
			February	=  2,
			March		=  3,
			April		=  4,
			May			=  5,
			June		=  6,
			July		=  7,
			August		=  8,
			September	=  9,
			October		= 10,
			November	= 11,
			December	= 12
		};

		enum WeekDay
		{
			Monday		= 1,
			Tuesday		= 2,
			Wednesday	= 3,
			Thursday	= 4,
			Friday		= 5,
			Saturday	= 6,
			Sunday		= 7
		};

		enum Component
		{
			ExtendedJulianPeriod,	// julian_date * milliseconds_per_day + milliseconds_since_day_start
			UnixTime,				// seconds since 1970

			Day,					// days / julian date
			Millisecond				// milliseconds
		};

		typedef Int64 Integral;

		struct Value
		{
			Integral year;
			Integral month;
			Integral day;

			Integral quarter;
			Integral week_day;
			Integral week;
			Integral week_year; // year of the week value (i.e 01.01.2010 as week = 53, and week_year=2009)
			Integral year_day;

			bool leap_year;

			Integral hour;
			Integral minute;
			Integral second;
			Integral millisecond;
		};

		static Integral const milliseconds_per_day = 1000 * 60 * 60 * 24;
		static Integral const julian_date_1970 = 2440588;

		Timestamp();
		Timestamp(Integral const value, Component const component = ExtendedJulianPeriod);

		void setDate(Integral const julian_period);
		void setDate(Integral const year, Integral const month, Integral const day);
		void setDate(Integral const year, Integral const year_day);
		void setTime(Integral const value);
		void setTime(Integral const hour, Integral const minute, Integral const second, Integral const millisecond = 0);

		Integral julianDay() { return m_date; }
		Value const& value() const { return m_value; }

		// Supports Day and Milliseconds
		Timestamp Add(Component const& component, Integral value) const;

		bool isLeapYear(Integral const year) const;
		Integral monthDays(Integral const month, bool leapYear = false) const;
		Integral yearDays(Integral const year) const;

		//Julian Period
		Integral date() const { return m_date; }

		// Milisseconds since the day start
		Integral time() const { return m_time; }

		Integral extendedJulianPeriod() const;


		//
		// Output mehtods
		//
		std::string abbreviatedWeekDay(Integral const week_day) const;
		std::string fullWeekDay(Integral const week_day) const;
		std::string abbreviatedMonth(Integral const month) const;
		std::string fullMonth(Integral const month) const;

		// Description
		//	Date string representation. Available patterns elements 
		//
		//	%a - Abbreviated weekday name
		//	%A - Full weekday name
		//	%b - Abbreviated month name
		//	%B - Full month name
		//	%c - Date and time {%x %X} [Different Behaviour]
		//	%d - Day of month as decimal number (01 – 31)
		//	%H - Hour in 24-hour format (00 – 23)
		//	%I - Hour in 12-hour format (01 - 12) [Not implemented]
		//	%j - Day of year as decimal number (001 – 366)
		//	%m - Month as decimal number (01 – 12)
		//	%M - Minute as decimal number (00 – 59)
		//	%p - AM/PM designation [Not implemented]
		//	%S - Second as decimal number (00 – 59)
		//	%s - Milliseconds as decimal number (000 - 999) [Extension]
		//	%U - Week of year as decimal number, with Sunday as first day of week (00 – 53) [Not Implemented]
		//	%W - Week of year as decimal number, with Monday as first day of week (00 – 53) [Not Implemented]
		//	%w - Weekday as decimal number (0 – 6; Sunday is 0)
		//	%x - Date representation {%d-%m-%y}  [Different Behaviour]
		//	%X - Time representation {%H:%M:%S}  [Different Behaviour]
		//	%y - Year without century, as decimal number (00 – 99)
		//	%Y - Year with century, as decimal number

		std::string format(std::string pattern) const;

		std::string toString() const;
	private:
		Integral m_date;
		Integral m_time;
		Value m_value;

		void initialize();
		void computeValues();
	};


} // namespace rengine

#endif //__RENGINE_TIMESTAMP_H__
