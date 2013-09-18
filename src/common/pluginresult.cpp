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
  : _status(status),
    _messageType(PluginResult::MESSAGE_TYPE_STRING),
    _strMessage(PluginResult::_statusMessages[(int)status])
{

}

PluginResult::PluginResult( PluginResult::Status status, const Json::Value& json )
  : _status(status),
    _messageType(PluginResult::MESSAGE_TYPE_JSON)
{
  Json::FastWriter writer;
  _encodedMessage = writer.write(json);
}

PluginResult::~PluginResult()
{

}
