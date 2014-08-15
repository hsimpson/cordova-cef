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

#include "callbackcontext.h"
#include "application.h"
#include "humblelogging/api.h"
#include <sstream>

HUMBLE_LOGGER(logger, "CallbackContext");

CallbackContext::CallbackContext( const std::string& callbackId, CefRefPtr<Application> app )
  : _callbackId(callbackId),
    _app(app),
    _finished(false)
{

}

CallbackContext::~CallbackContext()
{

}

void CallbackContext::sendPluginresult( std::shared_ptr<const PluginResult> result )
{
  {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    if(_finished)
    {
      std::stringstream ss;
      ss << "Attempted to send a second callback for ID: %d" << _callbackId << "\nResult was: " << result->getMessage();
      HL_WARN(logger, ss.str());
      return;
    }
    else
    {
      _finished = !result->getKeepCallback();
    }
  }
  _app->sendPluginResult(result, _callbackId);
}

void CallbackContext::success( const Json::Value& message )
{
  sendPluginresult(std::make_shared<PluginResult>(PluginResult::OK, message));
}

void CallbackContext::success( const std::string& message )
{
  sendPluginresult(std::make_shared<PluginResult>(PluginResult::OK, message));
}

void CallbackContext::success( const std::vector<char>& message )
{
  std::shared_ptr<PluginResult> p(new PluginResult(PluginResult::OK, message));
  sendPluginresult(p);
}

void CallbackContext::success( int message )
{
  sendPluginresult(std::make_shared<PluginResult>(PluginResult::OK, message));
}

void CallbackContext::success()
{
  sendPluginresult(std::make_shared<PluginResult>(PluginResult::OK));
}

void CallbackContext::error( const Json::Value& message )
{
  sendPluginresult(std::make_shared<PluginResult>(PluginResult::ERROR_, message));
}

void CallbackContext::error( const std::string& message )
{
  sendPluginresult(std::make_shared<PluginResult>(PluginResult::ERROR_, message));
}

void CallbackContext::error(int message)
{
  sendPluginresult(std::make_shared<PluginResult>(PluginResult::ERROR_, message));
}


