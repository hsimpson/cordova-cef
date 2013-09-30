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

#include "pluginresult.h"
#include "json/json.h"

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <sstream>

const char* PluginResult::_statusMessages[] = {
  "No result",
  "OK",
  "Class not found",
  "Illegal access",
  "Instantiation error",
  "Malformed url",
  "IO error",
  "Invalid action",
  "JSON error",
  "Error"
};

PluginResult::PluginResult( PluginResult::Status status )
  : _keepCallback(false),
    _status(status),
    _messageType(PluginResult::MESSAGE_TYPE_STRING),
    _strMessage(PluginResult::_statusMessages[(int)status])
{
  _encodedMessage = Json::valueToQuotedString(_strMessage.c_str());
}

PluginResult::PluginResult(PluginResult::Status status, const std::string& message)
  : _keepCallback(false),
    _status(status),
    _messageType(PluginResult::MESSAGE_TYPE_STRING),
    _strMessage(message)
{
  _encodedMessage = Json::valueToQuotedString(_strMessage.c_str());
}


PluginResult::PluginResult( PluginResult::Status status, const Json::Value& json )
  : _keepCallback(false),
    _status(status),
    _messageType(PluginResult::MESSAGE_TYPE_JSON)
{
  Json::FastWriter writer;
  _encodedMessage = writer.write(json);
}

PluginResult::PluginResult(PluginResult::Status status, int i)
  : _keepCallback(false),
    _status(status),
    _messageType(PluginResult::MESSAGE_TYPE_NUMBER),
    _encodedMessage(std::to_string(i))
{

}

PluginResult::PluginResult(PluginResult::Status status, float f)
  : _keepCallback(false),
    _status(status),
    _messageType(PluginResult::MESSAGE_TYPE_NUMBER),
    _encodedMessage(std::to_string(f))
{

}

PluginResult::PluginResult(PluginResult::Status status, bool b)
  : _keepCallback(false),
    _status(status),
    _messageType(PluginResult::MESSAGE_TYPE_NUMBER),
    _encodedMessage(b ? "true" : "false")
{

}

PluginResult::PluginResult(PluginResult::Status status, const std::vector<char>& data, bool binaryString)
  : _keepCallback(false),
    _status(status),
    _messageType(binaryString ? MESSAGE_TYPE_BINARYSTRING : MESSAGE_TYPE_ARRAYBUFFER)
{
  using namespace boost::archive::iterators;

  std::stringstream os;
  typedef 
    insert_linebreaks<         // insert line breaks every 72 characters
      base64_from_binary<    // convert binary values ot base64 characters
        transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
          const char *,
          6,
          8
        >
      > 
      ,72
    > 
    base64_text; // compose all the above operations in to a new iterator

  std::copy(
    base64_text(&data[0]),
    base64_text(&data[0] + data.size()),
    ostream_iterator<char>(os)
  );
  _encodedMessage = os.str();
}

PluginResult::~PluginResult()
{

}
