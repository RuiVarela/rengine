#include "UnitTest/UnitTest.h"

#include <rengine/lang/Lang.h>
#include <rengine/time/Timestamp.h>

#include <iostream>

using namespace rengine;

//
// UnitTestTimestamp
//

UNITT_TEST_BEGIN_CLASS(UnitTestTimestamp)

	virtual void run()
	{
		Timestamp::Value value;
		value.year = 2010;
		value.month = 11;
		value.day = 24;

		Timestamp timestamp;
		timestamp.setDate(value.year, value.month, value.day);
		UNITT_FAIL_NOT_EQUAL(value.year, timestamp.value().year);
		UNITT_FAIL_NOT_EQUAL(value.month, timestamp.value().month);
		UNITT_FAIL_NOT_EQUAL(value.day, timestamp.value().day);


		value.year = 2010;
		value.month = 1;
		value.day = 1;
		timestamp.setDate(value.year, value.month, value.day);
		UNITT_FAIL_NOT_EQUAL(value.year, timestamp.value().year);
		UNITT_FAIL_NOT_EQUAL(value.month, timestamp.value().month);
		UNITT_FAIL_NOT_EQUAL(value.day, timestamp.value().day);


		for (int i = 0; i != 365; ++i)
		{
			UNITT_FAIL_NOT_EQUAL(i + 1, timestamp.value().year_day);
			timestamp = timestamp.Add(Timestamp::Day, 1);
		}

		timestamp.setDate(value.year, value.month, value.day);
		UNITT_FAIL_NOT_EQUAL(53, timestamp.value().week);
		UNITT_FAIL_NOT_EQUAL(2009, timestamp.value().week_year);

		timestamp = timestamp.Add(Timestamp::Day, 7);
		UNITT_FAIL_NOT_EQUAL(1, timestamp.value().week);
		UNITT_FAIL_NOT_EQUAL(2010, timestamp.value().week_year);

		//
		// Week Day
		//
		value.year = 2010;
		value.month = 11;
		value.day = 21;
		timestamp.setDate(value.year, value.month, value.day);
		UNITT_FAIL_NOT_EQUAL(value.year, timestamp.value().year);
		UNITT_FAIL_NOT_EQUAL(value.month, timestamp.value().month);
		UNITT_FAIL_NOT_EQUAL(value.day, timestamp.value().day);
		UNITT_FAIL_NOT_EQUAL(Timestamp::Sunday, timestamp.value().week_day);

		timestamp = timestamp.Add(Timestamp::Day, 1);
		UNITT_FAIL_NOT_EQUAL(Timestamp::Monday, timestamp.value().week_day);

		timestamp = timestamp.Add(Timestamp::Day, 1);
		UNITT_FAIL_NOT_EQUAL(Timestamp::Tuesday, timestamp.value().week_day);

		timestamp = timestamp.Add(Timestamp::Day, 1);
		UNITT_FAIL_NOT_EQUAL(Timestamp::Wednesday, timestamp.value().week_day);

		timestamp = timestamp.Add(Timestamp::Day, 1);
		UNITT_FAIL_NOT_EQUAL(Timestamp::Thursday, timestamp.value().week_day);

		timestamp = timestamp.Add(Timestamp::Day, 1);
		UNITT_FAIL_NOT_EQUAL(Timestamp::Friday, timestamp.value().week_day);

		timestamp = timestamp.Add(Timestamp::Day, 1);
		UNITT_FAIL_NOT_EQUAL(Timestamp::Saturday, timestamp.value().week_day);

		timestamp = timestamp.Add(Timestamp::Day, 1);
		UNITT_FAIL_NOT_EQUAL(Timestamp::Sunday, timestamp.value().week_day);

		//
		// Quarter
		//
		timestamp.setDate(2010, 1, 1);
		UNITT_FAIL_NOT_EQUAL(1, timestamp.value().quarter);
		timestamp.setDate(2010, 2, 1);
		UNITT_FAIL_NOT_EQUAL(1, timestamp.value().quarter);
		timestamp.setDate(2010, 3, 1);
		UNITT_FAIL_NOT_EQUAL(1, timestamp.value().quarter);

		timestamp.setDate(2010, 4, 1);
		UNITT_FAIL_NOT_EQUAL(2, timestamp.value().quarter);
		timestamp.setDate(2010, 5, 1);
		UNITT_FAIL_NOT_EQUAL(2, timestamp.value().quarter);
		timestamp.setDate(2010, 6, 1);
		UNITT_FAIL_NOT_EQUAL(2, timestamp.value().quarter);

		timestamp.setDate(2010, 7, 1);
		UNITT_FAIL_NOT_EQUAL(3, timestamp.value().quarter);
		timestamp.setDate(2010, 8, 1);
		UNITT_FAIL_NOT_EQUAL(3, timestamp.value().quarter);
		timestamp.setDate(2010, 9, 1);
		UNITT_FAIL_NOT_EQUAL(3, timestamp.value().quarter);

		timestamp.setDate(2010, 10, 1);
		UNITT_FAIL_NOT_EQUAL(4, timestamp.value().quarter);
		timestamp.setDate(2010, 11, 1);
		UNITT_FAIL_NOT_EQUAL(4, timestamp.value().quarter);
		timestamp.setDate(2010, 12, 1);
		UNITT_FAIL_NOT_EQUAL(4, timestamp.value().quarter);


		//
		// output
		//
		timestamp = Timestamp(212160285444516);

		value.year = 2010;
		value.month = 12;
		value.day = 27;
		value.quarter = 4;
		value.week_day = 1;
		value.week = 52;
		value.week_year = 2010;
		value.year_day = 361;
		value.leap_year = false;
		value.hour = 20;
		value.minute = 37;
		value.second = 24;
		value.millisecond = 516;

		UNITT_FAIL_NOT_EQUAL(value.year, timestamp.value().year);
		UNITT_FAIL_NOT_EQUAL(value.month, timestamp.value().month);
		UNITT_FAIL_NOT_EQUAL(value.day, timestamp.value().day);
		UNITT_FAIL_NOT_EQUAL(value.quarter, timestamp.value().quarter);
		UNITT_FAIL_NOT_EQUAL(value.week_day, timestamp.value().week_day);
		UNITT_FAIL_NOT_EQUAL(value.week, timestamp.value().week);
		UNITT_FAIL_NOT_EQUAL(value.week_year, timestamp.value().week_year);
		UNITT_FAIL_NOT_EQUAL(value.year_day, timestamp.value().year_day);
		UNITT_FAIL_NOT_EQUAL(value.leap_year, timestamp.value().leap_year);
		UNITT_FAIL_NOT_EQUAL(value.hour, timestamp.value().hour);
		UNITT_FAIL_NOT_EQUAL(value.minute, timestamp.value().minute);
		UNITT_FAIL_NOT_EQUAL(value.second, timestamp.value().second);
		UNITT_FAIL_NOT_EQUAL(value.millisecond, timestamp.value().millisecond);

		UNITT_FAIL_NOT_EQUAL("Mon", timestamp.format("%a"));
		UNITT_FAIL_NOT_EQUAL("Monday", timestamp.format("%A"));
		UNITT_FAIL_NOT_EQUAL("Dec", timestamp.format("%b"));
		UNITT_FAIL_NOT_EQUAL("December", timestamp.format("%B"));
		UNITT_FAIL_NOT_EQUAL("27", timestamp.format("%d"));
		UNITT_FAIL_NOT_EQUAL("20", timestamp.format("%H"));
		UNITT_FAIL_NOT_EQUAL("361", timestamp.format("%j"));
		UNITT_FAIL_NOT_EQUAL("12", timestamp.format("%m"));
		UNITT_FAIL_NOT_EQUAL("37", timestamp.format("%M"));
		UNITT_FAIL_NOT_EQUAL("24", timestamp.format("%S"));
		UNITT_FAIL_NOT_EQUAL("516", timestamp.format("%s"));
		UNITT_FAIL_NOT_EQUAL("1", timestamp.format("%w"));
		UNITT_FAIL_NOT_EQUAL("10", timestamp.format("%y"));
		UNITT_FAIL_NOT_EQUAL("2010", timestamp.format("%Y"));


		UNITT_FAIL_NOT_EQUAL("27-12-10 20:37:24", timestamp.format("%c"));
		UNITT_FAIL_NOT_EQUAL("27-12-10", timestamp.format("%x"));
		UNITT_FAIL_NOT_EQUAL("20:37:24", timestamp.format("%X"));

//		std::cout << Timestamp().format("%c:%s") << std::endl;
//		std::cout << "day " << timestamp.value().day << " "
//				  << "month " << timestamp.value().month << " "
//				  << "year " << timestamp.value().year << " "
//				  << "hour " << timestamp.value().hour << " "
//				  << "minute " << timestamp.value().minute << " "
//				  << "second " << timestamp.value().second << " "
//				  << "millisecond " << timestamp.value().millisecond << " "
//				  << "week_day " << timestamp.value().week_day << " "
//				  << "year_day " << timestamp.value().year_day << " "
//				  << "year_week " << timestamp.value().year_week << " "
//				  << "quarter " << timestamp.value().quarter << " "
//				  << "leap_year " << timestamp.value().leap_year << std::endl;

	}

UNITT_TEST_END_CLASS(UnitTestTimestamp)
