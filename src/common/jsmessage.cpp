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

#include "jsmessage.h"
#include <sstream>

JsMessage::JsMessage( const std::string& js )
  :_jsPayloadOrCallbackId(js)
{

}

JsMessage::JsMessage( std::shared_ptr<const PluginResult> pluginResult, const std::string& callbackId )
  :_jsPayloadOrCallbackId(callbackId),
   _pluginResult(pluginResult)
{

}

JsMessage::~JsMessage()
{

}

int JsMessage::calculateEncodedLength() const
{
  if(_pluginResult.get() == nullptr)
    return _jsPayloadOrCallbackId.length() + 1;

  int statusLen = std::to_string((int)_pluginResult->getStatus()).length();
  int ret = 2 + statusLen + 1 + _jsPayloadOrCallbackId.length() + 1;
  switch (_pluginResult->getMessageType())
  {
    case PluginResult::MESSAGE_TYPE_BOOLEAN: // f or t
    case PluginResult::MESSAGE_TYPE_NULL: // N
      ret += 1;
      break;
    case PluginResult::MESSAGE_TYPE_NUMBER: // n
      ret += 1 + _pluginResult->getMessage().length();
      break;
    case PluginResult::MESSAGE_TYPE_STRING: // s
      ret += 1 + _pluginResult->getStrMessage().length();
      break;
    case PluginResult::MESSAGE_TYPE_BINARYSTRING:
      ret += 1 + _pluginResult->getMessage().length();
      break;
    case PluginResult::MESSAGE_TYPE_ARRAYBUFFER:
      ret += 1 + _pluginResult->getMessage().length();
      break;
    case PluginResult::MESSAGE_TYPE_JSON:
    default:
      ret += _pluginResult->getMessage().length();   
      break;
  }
  return ret;
}

void JsMessage::encodeAsJsMessage( std::stringstream& s ) const
{
  if(_pluginResult.get() == nullptr)
  {
    s << _jsPayloadOrCallbackId;
  }
  else
  {
    PluginResult::Status status = _pluginResult->getStatus();
    bool success = (status == PluginResult::OK) || (status == PluginResult::NO_RESULT);
    s << "cordova.callbackFromNative('"
      << _jsPayloadOrCallbackId      
      << "',"
      << (success ? "true" : "false")
      << ","
      << status
      << ",["
      << _pluginResult->getMessage() 
      << "],"
      << (_pluginResult->getKeepCallback() ? "true" : "false")
      << ");";
  }
}
