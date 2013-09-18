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

#include "application.h"






Application::Application(CefRefPtr<CefClient> client)
  : INIT_LOGGER(Application),
    _client(client),
    _appDirFetched(false)
{
}

Application::~Application()
{

}



void Application::OnContextInitialized()
{  
  // create plugin manager
  _pluginManager = new PluginManager(this);

  // load and parse config and fill configured plugins
  _config = new Config(getAppDirectory() + L"/config.xml", _pluginManager);
  
  // init plugin manager (also create the plugins with onload=true)
  _pluginManager->init();

  _startupUrl = L"file:///" + getAppDirectory() + L"/www/" + _config->startDocument(); 

  CefWindowInfo info;
  info.SetAsPopup(NULL, _config->appName());

  CefBrowserSettings browserSettings;
  browserSettings.developer_tools = STATE_ENABLED;
  browserSettings.file_access_from_file_urls = STATE_ENABLED;
  browserSettings.universal_access_from_file_urls = STATE_ENABLED;
  browserSettings.web_security = STATE_DISABLED;

  // Create the browser asynchronously and load the startup url
  BOOST_LOG_SEV(logger(), debug) << "create browser with startup url: '" << _startupUrl << "'";
  CefBrowserHost::CreateBrowser(info, _client, _startupUrl, browserSettings);
}

void Application::OnContextCreated( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context )
{
  CefRefPtr<CefV8Value> global = context->GetGlobal();
  _exposedJSObject = CefV8Value::CreateObject(NULL);
  CefRefPtr<CefV8Value> exec = CefV8Value::CreateFunction("exec", this);
  _exposedJSObject->SetValue("exec", exec, V8_PROPERTY_ATTRIBUTE_READONLY);
  global->SetValue("_cordovaNative", _exposedJSObject, V8_PROPERTY_ATTRIBUTE_READONLY);
}

void Application::OnContextReleased( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context )
{
  
}

bool Application::Execute( const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception )
{
  if(name == "exec" && arguments.size() == 4)
  {
    std::string service = arguments[0]->GetStringValue().ToString();
    std::string action  = arguments[1]->GetStringValue().ToString();
    std::string callbackId = arguments[2]->GetStringValue().ToString();
    std::string rawArgs = arguments[3]->GetStringValue().ToString();
    _pluginManager->exec(service, action, callbackId, rawArgs);
    return true;
  }
  return false;
}

void Application::sendJavascript( const std::string& statement )
{
  // ToDo:
}

void Application::sendPluginResult( const PluginResult& result, const std::string& callbackId )
{
  // ToDo:
}
