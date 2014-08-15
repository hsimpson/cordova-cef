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

#include "include/cef_runnable.h"
#include "application.h"
#include "client.h"
#include "humblelogging/api.h"



HUMBLE_LOGGER(logger, "Application");

Application::Application(std::shared_ptr<Helper::PathManager> pathManager)
  : CefApp(),
    _pathManager(pathManager),
    _mainWindow(NULL)
{
  _jsMessageQueue = std::make_shared<NativeToJsMessageQueue>(this);
  _pluginManager = std::make_shared<PluginManager>(this);

  // load and parse config and fill configured plugins
  Helper::Path config_path = _pathManager->getApplicationDir();
  config_path += "config.xml";
  _config = std::make_shared<Config>(config_path, _pluginManager);
}

Application::~Application()
{

}



void Application::OnContextInitialized()
{

}


void Application::createBrowser()
{
  std::string startup_document = _config->startDocument();

  // test if absolute url with http or https
  if(Helper::StringUtils::starts_with(startup_document, "http://") || Helper::StringUtils::starts_with(startup_document, "https://"))
  {
    _startupUrl = startup_document;
  }
  else
  {
    Helper::Path www_dir = _pathManager->getApplicationDir() + "www";
    std::string www_dir_str = www_dir.filePath();
    if(!Helper::StringUtils::starts_with(www_dir_str, "/"))
      www_dir_str = "/" + www_dir_str;
    _startupUrl = "file://" + www_dir_str + "/" + startup_document;
  }




  CefWindowInfo info;
  bool transparent = true;

  //bool offscreenrendering = false;
  //config()->getBoolPreference("OffScreenRendering", offscreenrendering);

  /*
  if(offscreenrendering)
  {
    CefRefPtr<Client::RenderHandler> osr_window = createOSRWindow(_mainWindow, _client.get(), transparent);
    _client->setOSRHandler(osr_window);
    // old
    //info.SetTransparentPainting(transparent ? true : false);
    //info.SetAsOffScreen(osr_window->handle());
    info.SetAsWindowless(osr_window->handle(), transparent);
  }
  else*/
  {
    RECT rect;
    GetClientRect(_mainWindow, &rect);
    info.SetAsChild(_mainWindow, rect);
  }

  /*
  RECT r;
  r.left = 0; r.top = 0; r.right = 700; r.bottom = 500;
  info.SetAsChild(_mainWindow, r);
  */

  //TODO: move the settings into config.xml
  CefBrowserSettings browserSettings;

  //browserSettings.developer_tools = STATE_ENABLED;
  browserSettings.file_access_from_file_urls = STATE_ENABLED;
  browserSettings.universal_access_from_file_urls = STATE_ENABLED;
  browserSettings.web_security = STATE_DISABLED;
  browserSettings.application_cache = STATE_ENABLED;
  browserSettings.local_storage = STATE_ENABLED;
  //browserSettings.accelerated_compositing = STATE_ENABLED; ??

  // init plugin manager (also create the plugins with onload=true)
  _pluginManager->init();

  // Create the browser asynchronously and load the startup url
  HL_DEBUG(logger, "create browser with startup url: '" + _startupUrl + "'");
  CefBrowserHost::CreateBrowser(info, _client.get(), _startupUrl, browserSettings, NULL);
}


void Application::OnContextCreated( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context )
{
  HL_DEBUG(logger, "create native JS bridge object");
  CefRefPtr<CefV8Value> global = context->GetGlobal();
  _exposedJSObject = CefV8Value::CreateObject(NULL);
  CefRefPtr<CefV8Value> exec = CefV8Value::CreateFunction("exec", this);
  _exposedJSObject->SetValue("exec", exec, V8_PROPERTY_ATTRIBUTE_READONLY);
  global->SetValue("_cordovaNative", _exposedJSObject, V8_PROPERTY_ATTRIBUTE_READONLY);
}

void Application::OnContextReleased( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context )
{

}


void Application::OnBrowserDestroyed( CefRefPtr<CefBrowser> browser )
{
  _pluginManager.reset();
  _jsMessageQueue.reset();
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
  _jsMessageQueue->addJavaScript(statement);
}

void Application::sendPluginResult( std::shared_ptr<const PluginResult> pluginResult, const std::string& callbackId )
{
  _jsMessageQueue->addPluginResult(pluginResult, callbackId);
}

void Application::runJavaScript( const std::string& js )
{      
  // ToDo: call the cordova.xxxx JS functions directly via the context


  // if there is no _exposedJSObject do nothing
  if(_exposedJSObject.get())
  {
    //if (CefCurrentlyOn(TID_UI))
    {
      CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
      CefRefPtr<CefFrame> frame = context->GetFrame();
      frame->ExecuteJavaScript(js, frame->GetURL(), 0);
    }
    //else 
    //{
    //  CefPostTask(TID_UI, NewCefRunnableMethod(this, &Application::runJavaScript, js));
    //}
  }
}

void Application::handlePause()
{
  runJavaScript("cordova.fireDocumentEvent('pause');");
}

void Application::handleResume()
{
  runJavaScript("cordova.fireDocumentEvent('resume');");
}

void Application::OnBeforeCommandLineProcessing( const CefString& process_type, CefRefPtr<CefCommandLine> command_line )
{
  command_line->AppendSwitch("allow-file-access-from-files");
}
