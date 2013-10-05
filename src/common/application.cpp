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
#include "client.h"

Application::Application(CefRefPtr<CefClient> client, std::shared_ptr<Helper::Paths> paths)
  : INIT_LOGGER(Application),
    _client(client),
    _paths(paths),    
    _jsMessageQueue(this)
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
  boost::filesystem::path config_path = _paths->getApplicationDir();
  config_path /= "config.xml";
  _config = new Config(config_path, _pluginManager);
  
  // init plugin manager (also create the plugins with onload=true)
  _pluginManager->init();

  boost::filesystem::path startup_document = _paths->getApplicationDir();
  startup_document /= "www";
  startup_document /= _config->startDocument();
  _startupUrl = "file:///" + startup_document.generic_string(); 

  CefWindowInfo info;
  info.SetAsPopup(NULL, _config->appName());

  CefBrowserSettings browserSettings;
  //browserSettings.developer_tools = STATE_ENABLED;
  browserSettings.file_access_from_file_urls = STATE_ENABLED;
  browserSettings.universal_access_from_file_urls = STATE_ENABLED;
  browserSettings.web_security = STATE_DISABLED;

  // Create the browser asynchronously and load the startup url
  LOG_DEBUG(logger()) << "create browser with startup url: '" << _startupUrl << "'";
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
  _jsMessageQueue.addJavaScript(statement);
}

void Application::sendPluginResult( std::shared_ptr<const PluginResult> pluginResult, const std::string& callbackId )
{
  _jsMessageQueue.addPluginResult(pluginResult, callbackId);
}

void Application::runJavaScript( const std::string& js )
{
  Client* c = dynamic_cast<Client*>(_client.get());
  if(c)
    c->runJavaScript(js);
}
