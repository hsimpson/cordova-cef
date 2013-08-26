/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
*/

#ifndef logging_h__
#define logging_h__




#include <boost/shared_ptr.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/support/date_time.hpp> 

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

// We define our own severity levels

inline void init_logging()
{
  boost::shared_ptr< logging::core > core = logging::core::get();

  boost::shared_ptr< sinks::text_file_backend > backend =
    boost::make_shared< sinks::text_file_backend >(
    keywords::file_name = "x:\\file_%5N.log",                                          
    keywords::rotation_size = 5 * 1024 * 1024,                                     
    keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0),
    keywords::auto_flush = true,
    keywords::format = "[%TimeStamp%]: %Message%"
    );

  // Wrap it into the frontend and register in the core.
  // The backend requires synchronization in the frontend.
  typedef sinks::synchronous_sink< sinks::text_file_backend > sink_t;
  boost::shared_ptr< sink_t > sink(new sink_t(backend));

  sink->set_formatter(
    expr::stream
    << "[" << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%d.%m.%Y %H:%M:%S.%f") << "] "
    << "[PID=" << expr::attr< attrs::current_process_id::value_type >("ProcessID") << "] "
    << "[TID=" << expr::attr< attrs::current_thread_id::value_type >("ThreadID") << "] "
    /*<< "[" << expr::attr< std::string >("Channel") << "] "*/
    << "[" <<  logging::trivial::severity  << "] "
    << "[" << expr::attr<std::string>("Channel") << "]: "
    << expr::smessage
  );

  core->add_sink(sink);

  // Register common attributes
  logging::add_common_attributes();
  
}

using namespace logging::trivial;
// declare global logger(s)
BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(
  g_appLogger,
  src::severity_channel_logger< severity_level >,
  (keywords::severity = debug)
  (keywords::channel = "app")
)

BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(
  g_clientLogger,
  src::severity_channel_logger< severity_level >,
  (keywords::severity = debug)
  (keywords::channel = "client")
)

#define DECLARE_LOGGER(clazzname)\
  private:\
    typedef src::severity_channel_logger_mt<severity_level> logger_type;\
    logger_type _logger;\
    logger_type& logger() {return _logger;}

#define STR(s) #s

#define INIT_LOGGER(clazzname)\
  _logger(keywords::channel = STR(clazzname))

#endif // logging_h__
