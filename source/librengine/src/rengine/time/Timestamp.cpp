#include <rengine/string/String.h>
#include <rengine/time/Timestamp.h>
#include <rengine/lang/debug/Debug.h>

#include <sys/timeb.h>
#include <cstring>
#include <vector>
#include <iomanip>

namespace rengine
{
	Timestamp::Timestamp()
	{
		initialize();

		// 1970
		Integral milliseconds = julian_date_1970 * milliseconds_per_day;

#if RENGINE_COMPILER == RENGINE_COMPILER_MSVC
		struct _timeb tp;
		_ftime(&tp);
#endif //RENGINE_COMPILER == RENGINE_COMPILER_GNUC


#if RENGINE_COMPILER == RENGINE_COMPILER_GNUC
		struct timeb tp;
		ftime(&tp);
#endif //RENGINE_COMPILER == RENGINE_COMPILER_GNUC

		milliseconds += tp.time * 1000 + tp.millitm;
	
		m_date = (milliseconds / milliseconds_per_day);
		m_time = (milliseconds % milliseconds_per_day);
		computeValues();
	}

	Timestamp::Timestamp(Integral const value, Component const component)
	{
		initialize();

		if (component == ExtendedJulianPeriod)
		{
			m_date = (value / milliseconds_per_day);
			m_time = (value % milliseconds_per_day);
		}
		else if (component == UnixTime)
		{
			Timestamp year_1970;
			year_1970.setDate(1970, 1, 1);

			Integral milliseconds = (julian_date_1970 * milliseconds_per_day) + (value * 1000);

			m_date = (milliseconds / milliseconds_per_day);
			m_time = (milliseconds % milliseconds_per_day);
		}
		else if (component == Day)
		{
			m_date = value;
		}
		else if (component == Millisecond)
		{
			m_time = (value);
		}

		computeValues();
	}

	void Timestamp::initialize()
	{
		m_date = 0;
		m_time = 0;
		memset(&m_value, 0, sizeof(Value));
	}

	Timestamp::Integral Timestamp::extendedJulianPeriod() const
	{
		return m_time + milliseconds_per_day * m_date;
	}

	void Timestamp::setTime(Integral const value)
	{
		RENGINE_ASSERT(value > 0);

		m_time = value;
		computeValues();
	}

	void Timestamp::setTime(Integral const hour, Integral const minute, Integral const second, Integral const millisecond)
	{
		RENGINE_ASSERT(hour >= 0);
		RENGINE_ASSERT(hour < 24);

		RENGINE_ASSERT(minute >= 0);
		RENGINE_ASSERT(minute < 60);

		RENGINE_ASSERT(second >= 0);
		RENGINE_ASSERT(second < 60);

		RENGINE_ASSERT(millisecond >= 0);
		RENGINE_ASSERT(millisecond < 1000);

		m_time = millisecond + 1000 * (second + 60 * (minute + 60 *hour));
		computeValues();
	}

	void Timestamp::setDate(Integral const julian_period)
	{
		RENGINE_ASSERT(julian_period > 0);

		m_date = julian_period;
		computeValues();
	}

	void Timestamp::setDate(Integral const year, Integral const month, Integral const day)
	{
		RENGINE_ASSERT(month > 0);
		RENGINE_ASSERT(month < 13);

		RENGINE_ASSERT(day > 0);
		RENGINE_ASSERT(day <= monthDays(month, isLeapYear(year)));


		Integral a = (14 - month) / 12;
		Integral y = year + 4800 - a;
		Integral m = month + 12 * a - 3;
		m_date = day + (153 * m + 2) / 5 + y * 365 + y / 4 - y / 100 + y / 400 - 32045;

		computeValues();
	}

	void Timestamp::setDate(Integral const year, Integral const year_day)
	{
		RENGINE_ASSERT(year_day > 0);
		RENGINE_ASSERT(year_day <= yearDays(year));

		Integral y = year + 4799;
		m_date = year_day + y * 365 + y / 4 - y / 100 + y / 400 - 31739;

		computeValues();
	}

	Timestamp Timestamp::Add(Component const& component, Integral value) const
	{
		Timestamp output = *this;

		if (component == Day)
		{
			value += m_date;

			if (value < 0)
			{
				value = 0;
			}
			return Timestamp(value, component);
		}
		else if (component == Millisecond)
		{
			value += extendedJulianPeriod();
			if (value < 0)
			{
				value = 0;
			}

			return Timestamp(value, ExtendedJulianPeriod);
		}

		return *this;
	}

	void Timestamp::computeValues()
	{
		Integral a = m_date + 32044;
		Integral b = (4 * a + 3) / 146097;
		Integral c = a - (b * 146097)/4;

		Integral d = (4 * c + 3) / 1461;
		Integral e = c - (1461 * d) / 4;
		Integral m = (5 * e + 2) / 153;

		m_value.day = e - (153 * m + 2) / 5 + 1;
		m_value.month = m + 3 - 12 * (m / 10);
		m_value.year = b * 100 + d - 4800 + m / 10;

		Integral y = m_value.year + 4799;
		m_value.year_day = m_date - (y * 365 + y / 4 - y / 100 + y/400 - 31739);
		m_value.leap_year = isLeapYear(m_value.year);


		Integral d4 = (m_date + 31741 - (m_date % 7)) % 146097 % 36524 % 1461;
		Integral L  = d4 / 1460;
		Integral d1 = ((d4 - L) % 365) + L;
		m_value.week = d1 / 7 + 1;
		m_value.week_year = m_value.year + ((m_value.week == 1) & (m_value.month == 12)) - ((m_value.week > 51) & (m_value.month));
		m_value.week_day = m_date % 7 + 1;
		m_value.quarter = (m_value.month - 1) / 3 + 1;

		Integral time_value = m_time;

		m_value.millisecond = time_value % 1000;
		time_value /= 1000;

		m_value.second = time_value % 60;
		time_value /= 60;

		m_value.minute = time_value % 60;
		time_value /= 60;

		m_value.hour = time_value;
	}

	Timestamp::Integral Timestamp::yearDays(Integral const year) const
	{
		return (isLeapYear(year) ? 366 : 365);
	}

	bool Timestamp::isLeapYear(Integral const year) const
	{
		return bool((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0));
	}

	Timestamp::Integral Timestamp::monthDays(Integral const month, bool leapYear) const
	{
		RENGINE_ASSERT(month > 0);
		RENGINE_ASSERT(month < 13);

		Integral value = 0;

		switch (month)
		{
			case  1: value = 31; break;
			case  2: value = (leapYear ? 29 : 28) ; break;
			case  3: value = 31; break;
			case  4: value = 30; break;
			case  5: value = 31; break;
			case  6: value = 30; break;
			case  7: value = 31; break;
			case  8: value = 31; break;
			case  9: value = 30; break;
			case 10: value = 31; break;
			case 11: value = 30; break;
			case 12: value = 31; break;
			default: value = 0; break;
		}

		return value;
	}

	std::string Timestamp::abbreviatedWeekDay(Integral const week_day) const
	{
		RENGINE_ASSERT(week_day > 0);
		RENGINE_ASSERT(week_day < 8);
		return fullWeekDay(week_day).substr(0, 3);
	}

	std::string Timestamp::fullWeekDay(Integral const week_day) const
	{
		RENGINE_ASSERT(week_day > 0);
		RENGINE_ASSERT(week_day < 8);

		static char const* days[] = {	"Invalid",
										"Monday" ,
										"Tuesday" ,
										"Wednesday" ,
										"Thursday" ,
										"Friday" ,
										"Saturday" ,
										"Sunday" 
									};
		return days[week_day];
	}

	std::string Timestamp::abbreviatedMonth(Integral const month) const
	{
		RENGINE_ASSERT(month > 0);
		RENGINE_ASSERT(month < 13);
		return fullMonth(month).substr(0, 3);
	}

	std::string Timestamp::fullMonth(Integral const month) const
	{
		RENGINE_ASSERT(month > 0);
		RENGINE_ASSERT(month < 13);

		static char const* months[] = {	"Invalid",
										"January" ,
										"February" ,
										"March" ,
										"April" ,
										"May" ,
										"June" ,
										"July" ,
										"August" ,
										"September" ,
										"October" ,
										"November" ,
										"December" 
									};
		return months[month];
	}

	std::string Timestamp::toString() const
	{
		std::stringstream string;
		string	<< std::setfill('0')
				<< std::setw(2) << m_value.day << "-" 
				<< std::setw(2) << m_value.month << "-"
				<< std::setw(2) << m_value.year % 100 << " "
				<< std::setw(2) << m_value.hour << ":"
				<< std::setw(2) << m_value.minute << ":"
				<< std::setw(2) << m_value.second << ":"
				<< std::setw(2) << m_value.millisecond;
		return string.str();
	}

	std::string Timestamp::format(std::string pattern) const
	{
		typedef std::vector<std::string> MaskVector;
		
		MaskVector masks;
		masks.push_back("%a");	//	%a - Abbreviated weekday name
		masks.push_back("%A");	//	%A - Full weekday name
		masks.push_back("%b");	//	%b - Abbreviated month name
		masks.push_back("%B");	//	%B - Full month name
		masks.push_back("%c");	//	%c - Date and time
		masks.push_back("%d");	//	%d - Day of month as decimal number (01 – 31)
		masks.push_back("%H");	//	%H - Hour in 24-hour format (00 – 23)
		masks.push_back("%j");	//	%j - Day of year as decimal number (001 – 366)
		masks.push_back("%m");	//	%m - Month as decimal number (01 – 12)
		masks.push_back("%M");	//	%M - Minute as decimal number (00 – 59)
		masks.push_back("%S");	//	%S - Second as decimal number (00 – 59)
		masks.push_back("%s");	//	%s - Milliseconds as decimal number (000 - 999)
		masks.push_back("%w");	//	%w - Weekday as decimal number (0 – 6; Sunday is 0)
		masks.push_back("%x");	//	%x - Date representation
		masks.push_back("%X");	//	%X - Time representation
		masks.push_back("%y");	//	%y - Year without century, as decimal number (00 – 99)
		masks.push_back("%Y");	//	%Y - Year with century, as decimal number

		for (MaskVector::const_iterator i = masks.begin(); i != masks.end(); ++i)
		{
			if (pattern.find(*i) != std::string::npos)
			{			
				std::stringstream replace_string;
				replace_string << std::setfill('0');

				if (*i == "%a")			{ replace_string << abbreviatedWeekDay(m_value.week_day); }
				else if (*i == "%A")	{ replace_string << fullWeekDay(m_value.week_day); }
				else if (*i == "%b")	{ replace_string << abbreviatedMonth(m_value.month); }
				else if (*i == "%B")	{ replace_string << fullMonth(m_value.month); }
				else if (*i == "%c")	{ replace_string << std::setw(2) << m_value.day << "-" 
														 << std::setw(2) << m_value.month << "-"
														 << std::setw(2) << m_value.year % 100 << " "
														 << std::setw(2) << m_value.hour << ":"
														 << std::setw(2) << m_value.minute << ":"
														 << std::setw(2) << m_value.second;	} 
				else if (*i == "%d")	{ replace_string << std::setw(2) << m_value.day; } 
				else if (*i == "%H")	{ replace_string << std::setw(2) << m_value.hour; }  
				else if (*i == "%j")	{ replace_string << std::setw(3) << m_value.year_day; } 
				else if (*i == "%m")	{ replace_string << std::setw(2) << m_value.month; }  
				else if (*i == "%M")	{ replace_string << std::setw(2) << m_value.minute; }  
				else if (*i == "%S")	{ replace_string << std::setw(2) << m_value.second; } 
				else if (*i == "%s")	{ replace_string << std::setw(3) << m_value.millisecond; }  
				else if (*i == "%w")	{ replace_string << m_value.week_day % 7; }  
				else if (*i == "%x")	{ replace_string << stringPrinter("%02d-%02d-%02d", m_value.day, m_value.month, m_value.year % 100); } 
				else if (*i == "%X")	{ replace_string << stringPrinter("%02d:%02d:%02d", m_value.hour, m_value.minute, m_value.second); } 
				else if (*i == "%y")	{ replace_string << std::setw(2)  << m_value.year % 100; } 
				else if (*i == "%Y")	{ replace_string << m_value.year; } 

				replace(pattern, *i, replace_string.str());
			}
		}

		return pattern;
	}

} // namespace rengine
