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

#ifndef application_h__
#define application_h__

#include "include/cef_app.h"
#include "client.h"
#include "config.h"
#include "logging.h"
#include "pluginmanager.h"
#include "pluginresult.h"
#include "nativetojsmessagequeue.h"
#include "helper.h"


class Application : public CefApp,
                    public CefBrowserProcessHandler,
                    public CefRenderProcessHandler,
                    public CefV8Handler
{
public:
  Application(CefRefPtr<Client> client, std::shared_ptr<Helper::Paths>);
  virtual ~Application();

  // CefApp method(s)
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }
  virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this; }

  //CefBrowserProcessHandler method(s)
  virtual void OnContextInitialized() OVERRIDE;

  //CefRenderProcessHandler method(s)
  virtual void OnContextCreated( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context ) OVERRIDE;
  virtual void OnContextReleased( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context ) OVERRIDE;

  //CefV8Handler method(s)
  virtual bool Execute( const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception ) OVERRIDE;

  void sendJavascript(const std::string& statement);
  void sendPluginResult(std::shared_ptr<const PluginResult> pluginResult, const std::string& callbackId);
  void runJavaScript(const std::string& js);

  void Application::InitializeAppData();
  void Application::MoveWebAppToTempFs();

  std::shared_ptr<Helper::Paths> getPaths();

  CefWindowHandle getHandle();

  const CefRefPtr<Config> config() const {return _config;}

protected:

  virtual CefRefPtr<Client::RenderHandler> createOSRWindow(CefWindowHandle parent, OSRBrowserProvider* browser_provider, bool transparent) = 0;
  virtual void handlePause();
  virtual void handleResume();

  CefRefPtr<Client> _client;
  CefRefPtr<Config> _config;
  CefRefPtr<PluginManager> _pluginManager;

  std::string _startupUrl;

  CefRefPtr<CefV8Value> _exposedJSObject;

  NativeToJsMessageQueue _jsMessageQueue;
  std::shared_ptr<Helper::Paths> _paths;

  CefWindowHandle _mainWindow;
  
  IMPLEMENT_REFCOUNTING(Application);

  DECLARE_LOGGER(Application);
};
#endif // application_h__
