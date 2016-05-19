#include "UnitTest/UnitTest.h"

#include <rengine/outputstream/Log.h>
#include <rengine/time/PerformanceAnalyzer.h>
#include <rengine/thread/Thread.h>
#include <iostream>


using namespace rengine;


//
// UnitTestPerformanceAnalyzer
//

UNITT_TEST_BEGIN_CLASS(UnitTestPerformanceAnalyzer)

	virtual void run()
	{
		std::string log_name = "unit_test_performace_analyzer";
		
		UNITT_FAIL_NOT_EQUAL(0, PerformanceAnalyzer::numberOfQueries());
		UNITT_ASSERT(  !PerformanceAnalyzer::hasQuery("Simple Query #1") );
		PerformanceAnalyzer::Query query("Simple Query #1");
		for (unsigned int i = 0; i != 15; ++i)
		{
			query.start();
			RLOG_INFO(log_name, "Simple Query Test");
			query.end();
		}
		UNITT_FAIL_NOT_EQUAL(1, PerformanceAnalyzer::numberOfQueries());
		UNITT_ASSERT( PerformanceAnalyzer::hasQuery("Simple Query #1") );

		for (unsigned int i = 0; i != 10; ++i)
		{
			PerformanceAnalyzer::ScopedQuery query("Scoped Query #1");
			RLOG_INFO(log_name, "Scoped Query test" << i * i);
		}

		UNITT_FAIL_NOT_EQUAL(2, PerformanceAnalyzer::numberOfQueries());
		UNITT_ASSERT( PerformanceAnalyzer::hasQuery("Scoped Query #1") );


		//"Scoped Query #2"
		{
			float one = 1;
			float another = 1;
			PerformanceAnalyzer::ScopedQuery query("Scoped Query #2");
			for (unsigned int i = 0; i != 100; ++i)
			{
				one *= one;
				another *= another;
			
				one += i;
				another *= one;
			}
		}

		int sleep_time = 100 * 1000;
		for (unsigned int i = 0; i != 10; ++i)
		{
			PerformanceAnalyzer::ScopedQuery query("Sleep Test");
			Thread::microSleep(sleep_time);
		}

		for (unsigned int i = 0; i != 10; ++i)
		{
			PerformanceAnalyzer::DelayedQuery delayed_query("Delayed Query", 3000.0, 5000.0);
			delayed_query.start();
			RLOG_INFO(log_name, "Delayed Query");
			delayed_query.end();
		}

		{ 
			PerformanceAnalyzer::ScopedDelayedQuery scoped_delayed_query("Scoped Delayed Query", 5000.0);
			RLOG_INFO(log_name, "Scoped Delayed Query");
		}


		PerformanceAnalyzer::ToggleQuery toggle_query("Toggle Query");

		toggle_query.toggle();
		RLOG_INFO(log_name, "Toggle Delayed Query");
		toggle_query.toggle();
		RLOG_INFO(log_name, "Toggle Delayed Query");
		toggle_query.toggle();
		RLOG_INFO(log_name, "Toggle Delayed Query");
		toggle_query.toggle();


		PerformanceAnalyzer::MarkerQuery("Marker Query");
		RLOG_INFO(log_name, "Marker Query ");
		PerformanceAnalyzer::MarkerQuery("Marker Query");
		RLOG_INFO(log_name, "Marker Query ");
		PerformanceAnalyzer::MarkerQuery("Marker Query");
		RLOG_INFO(log_name, "Marker Query ");
		PerformanceAnalyzer::MarkerQuery("Marker Query");
		RLOG_INFO(log_name, "Marker Query ");
		PerformanceAnalyzer::MarkerQuery("Marker Query");

		PerformanceAnalyzer::analyzeQueries(LogDataAnalyzer(log_name));
		PerformanceAnalyzer::analyzeQueries(StatsLogDataAnalyzer(log_name));
		//PerformanceAnalyzer::analyzeQueries(StatsPrinterDataAnalyzer());
	}

UNITT_TEST_END_CLASS(UnitTestPerformanceAnalyzer)

