// __!!rengine_copyright!!__ //

#ifndef __RENGINE_PERFORMANCE_ANALYZER__
#define __RENGINE_PERFORMANCE_ANALYZER__

#include <rengine/time/Timer.h>
#include <rengine/outputstream/Log.h>

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <limits>
#include <iostream>

namespace rengine
{
	class PerformanceAnalyzer
	{
	public :
		struct QueryData
		{
			QueryData()
				:begin_time(0.0), end_time(0.0)
			{}

			QueryData(QueryData const& rhs)
			{
				begin_time = rhs.begin_time;
				end_time = rhs.end_time;
			}

			double begin_time;
			double end_time;
		};

		typedef std::vector<QueryData> QueryResultSet;
		typedef std::map<std::string, QueryResultSet> QueryResultSetMap;

		PerformanceAnalyzer() {}
		~PerformanceAnalyzer() {}

		static Real64 currentTime() { return timer_.elapsedTimeMilliseconds(); }
		static void beginTimming() { timer_.start(); }

		static unsigned int numberOfQueries() { return (unsigned int)(queries_.size()); }
		
		static void addQueryResult(std::string const& query_identifyer, QueryData const& query_data)
		{
			QueryResultSetMap::iterator query_map_position = queries_.find(query_identifyer);

			if (query_map_position == queries_.end())
			{
				queries_[query_identifyer] = QueryResultSet();
				queries_[query_identifyer].push_back(query_data);
			}
			else
			{
				query_map_position->second.push_back(query_data);
			}	
		}
		
		
		static std::string queryIdentifier(unsigned int index)
		{
			RENGINE_ASSERT( index < numberOfQueries() );
			RENGINE_ASSERT( index >= 0 );

			unsigned int counter = 0;
			std::string identifier;

			QueryResultSetMap::iterator iterator = queries_.begin();
			while(iterator != queries_.end())
			{
				if (counter == index)
				{
					identifier = iterator->first;
					break;
				}

				++iterator;
				++counter;
			}

			return identifier;
		}
		
		static void clearQuery(std::string const& query_identifyer)
		{		
			QueryResultSetMap::iterator query_map_position = queries_.find( query_identifyer );
			if (query_map_position != queries_.end())
			{
				queries_.erase(query_map_position);
			}
		}
		
		static void clear()
		{
			queries_.clear();
		}

		static bool hasQuery(std::string const& query_identifyer)
		{
			QueryResultSetMap::iterator query_map_position = queries_.find( query_identifyer );
			return (query_map_position != queries_.end());
		}

		static QueryResultSet const& queryResultSet(std::string const query_identifyer)
		{
			RENGINE_ASSERT( hasQuery(query_identifyer) );
			return queries_[ query_identifyer ];
		}

	
		template <typename T>
		static void analyzeQuery(std::string const& query_identifyer, T & analyzer)
		{
			assert( hasQuery(query_identifyer) );

			QueryResultSetMap::iterator query = queries_.find( query_identifyer );
			analyzer.begin(query->first);

			for (QueryResultSet::size_type i = 0; i != query->second.size(); ++i)
			{
				analyzer.operator() ( query->second[i] );
			}

			analyzer.end();
		}

		template <typename T>
		static void analyzeQueries(T & analyzer)
		{
			QueryResultSetMap::iterator iterator = queries_.begin();
			while(iterator != queries_.end())
			{
				analyzer.begin(iterator->first);

				for (QueryResultSet::size_type i = 0; i != iterator->second.size(); ++i)
				{
					analyzer.operator() ( iterator->second[i] );
				}

				analyzer.end();

				++iterator;
			}
		}
		
	private:
		static Timer timer_;
		static QueryResultSetMap queries_;

	public:
		
		class Query
		{
		public :
			Query() {}

			Query(std::string const &identifier)
				:identifier_(identifier)
			{}
			virtual ~Query() {};

			virtual void start() { query_data.begin_time = PerformanceAnalyzer::currentTime(); }
			virtual void end() 
			{
				query_data.end_time = PerformanceAnalyzer::currentTime(); 
				PerformanceAnalyzer::addQueryResult(identifier(), queryData());
			}

			std::string const& identifier() const { return identifier_; } 
			double startTime() const { return query_data.begin_time; }
			double endTime() const { return query_data.end_time; }
			QueryData const& queryData() const { return query_data; }
		private:
			std::string identifier_;
			QueryData query_data;
		};

		class DelayedQuery : public Query
		{
		public :
			DelayedQuery(std::string const &identifier,
				double const first_time = std::numeric_limits<double>::min(),
				double const last_time = std::numeric_limits<double>::max())
				:Query(identifier), first_time_(first_time), last_time_(last_time), do_timming_(false)
			{}

			virtual void start() 
			{
				double time = PerformanceAnalyzer::currentTime();

				do_timming_ = ((time >= first_time_) && (time <= last_time_));

				if (do_timming_)
				{
					Query::start();
				}
			}

			virtual void end() 
			{
				if (do_timming_)
				{
					Query::end();
				}
			}

		private :
			double first_time_;
			double last_time_;
			bool do_timming_;
		};

		class ToggleQuery : public Query
		{
		public :
			ToggleQuery() {}

			ToggleQuery(std::string const &identifier)
				:Query(identifier), do_start_(true)
			{}

			virtual void toggle()
			{
				if (do_start_)
				{
					start();
					do_start_ = false;
				}
				else
				{
					end();
					start();
				}
			}

		private :
			bool do_start_;
		};

		class ScopedQuery : public Query
		{
		public :
			ScopedQuery(std::string const &identifier)
				:Query(identifier)
			{
				start(); 
			}

			virtual ~ScopedQuery() { end(); }
		};

		class ScopedDelayedQuery : public DelayedQuery
		{
		public :
			ScopedDelayedQuery(std::string const &identifier,
				double const first_time = std::numeric_limits<double>::min(),
				double const last_time = std::numeric_limits<double>::max())
				:DelayedQuery(identifier, first_time, last_time)
			{
				start(); 
			}

			virtual ~ScopedDelayedQuery() { end(); }
		};

		class MarkerQuery 
		{
		public :
			typedef std::map<std::string, ToggleQuery> ToggleQueryMap;

			MarkerQuery(std::string const &identifier)
			{
				ToggleQueryMap::iterator element = toggle_queries_.find( identifier );

				if (element == toggle_queries_.end())
				{
					ToggleQuery toggle_query(identifier);
					toggle_query.toggle();

					toggle_queries_[identifier] = toggle_query;
				}
				else
				{
					element->second.toggle();
				}
			}

			~MarkerQuery() {}

			static void clear() { toggle_queries_.clear(); }
		private:
			static ToggleQueryMap toggle_queries_;
		};

	};


	//
	// Data Analyzer
	//


//
//	Model DataAnalyzer
//
//	void begin() called before a query analysis
//	void end() called after a query analysis
//	void operator() (PerformanceAnalyzer::QueryData const& query_data) // called for each data element in a query data set
//

	struct QueryCounterDataAnalyzer
	{
		typedef std::pair<std::string, int> StringIntPair;
		QueryCounterDataAnalyzer(){}

		void begin(std::string const& query_identifyer)
		{
			counter.push_back( StringIntPair(query_identifyer, 0) );		
		}

		void end() {}

		void operator() (PerformanceAnalyzer::QueryData const& query_data) 
		{ 
			counter.back().second++; 
		}

		std::vector<StringIntPair> counter;
	};

	struct StatsDataAnalyzer
	{
		struct StatsData
		{
			StatsData()
			{
				max_time = std::numeric_limits<double>::min();
				min_time = std::numeric_limits<double>::max();
				average_time = 0.0;
				total_time = 0.0;
				begin_time = std::numeric_limits<double>::max();
				end_time = std::numeric_limits<double>::min();
				counter = 0;
			}

			std::string identifyer;
			double max_time;
			double min_time;
			double average_time;
			double total_time;
			double begin_time;
			double end_time;
			unsigned int counter;

			PerformanceAnalyzer::QueryData max_element;
			PerformanceAnalyzer::QueryData min_element;

		};

		StatsDataAnalyzer(){}

		void begin(std::string const& query_identifyer)
		{
			StatsData stats;
			stats.identifyer = query_identifyer;
			querys.push_back(stats);		
		}

		void end() 
		{
			if (querys.back().counter > 0)
			{
				querys.back().average_time = querys.back().total_time / float(querys.back().counter);
			}
		}

		void operator() (PerformanceAnalyzer::QueryData const& query_data) 
		{ 
			double element_time = query_data.end_time - query_data.begin_time;

			if (query_data.begin_time < querys.back().begin_time)
			{
				querys.back().begin_time = query_data.begin_time;
			}

			if (query_data.end_time > querys.back().end_time)
			{
				querys.back().end_time = query_data.end_time;
			}

			if (element_time < querys.back().min_time)
			{
				querys.back().min_time = element_time;
				querys.back().min_element = query_data;
			}

			if (element_time > querys.back().max_time)
			{
				querys.back().max_time = element_time;
				querys.back().max_element = query_data;
			}

			querys.back().total_time += element_time;
			querys.back().counter++; 
		}

		std::vector<StatsData> querys;
	};



	// Printers
	struct StatsPrinterDataAnalyzer : public StatsDataAnalyzer
	{
		StatsPrinterDataAnalyzer(std::ostream& out = std::cout) : os(out) {}

		void end()
		{
			StatsDataAnalyzer::end();

			os << "[" << querys.size() << "] " << querys.back().identifyer << " Total Queries : " << querys.back().counter << std::endl;
			os << "[" << querys.size() << "] " << querys.back().identifyer << " Running Time : [" << querys.back().begin_time << " - " << querys.back().end_time << "] " << querys.back().total_time << std::endl;
			os << "[" << querys.size() << "] " << querys.back().identifyer << " Min : " << querys.back().min_time << " Max : " << querys.back().max_time << " " << " Average : " <<  querys.back().average_time << std::endl;
		}

		std::ostream& os;
	};

	struct StatsLogDataAnalyzer : public StatsDataAnalyzer
	{
		StatsLogDataAnalyzer(std::string const& name) : log_name(name) {}

		void end()
		{
			StatsDataAnalyzer::end();
			RLOG_INFO(log_name, "[" << querys.size() << "] " << querys.back().identifyer << " Total Queries : " << querys.back().counter);
			RLOG_INFO(log_name, "[" << querys.size() << "] " << querys.back().identifyer << " Running Time : [" << querys.back().begin_time << " - " << querys.back().end_time << "] " << querys.back().total_time);
			RLOG_INFO(log_name, "[" << querys.size() << "] " << querys.back().identifyer << " Min : " << querys.back().min_time << " Max : " << querys.back().max_time << " " << " Average : " <<  querys.back().average_time);
		}

		std::string log_name;
	};

	struct LogDataAnalyzer
	{
		LogDataAnalyzer(std::string const& name) : log_name(name),  count(0) {}

		void begin(std::string const& query_identifyer)
		{
			identifyer = query_identifyer;
			count = 0;
		}

		void end() {}

		void operator() (PerformanceAnalyzer::QueryData const& query_data) 
		{ 
			RLOG_INFO(log_name, identifyer << " [" << count << "] " 
			   << query_data.end_time - query_data.begin_time
			   << " [" << query_data.begin_time << " - " << query_data.end_time << "]"); 
			++count; 
		}

		std::string log_name;
		int count;
		std::string identifyer;
	};








} //namespace rengine

#endif //__RENGINE_PERFORMANCE_ANALYZER__
