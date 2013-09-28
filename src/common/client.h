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

#ifndef client_h__
#define client_h__


#include "include/cef_client.h"
#include <list>
#include <set>
#include <string>
#include "common/logging.h"

class Client : public CefClient,
                      CefLifeSpanHandler,
                      CefKeyboardHandler
{
public:
  Client();
  virtual ~Client();

  // CefClient method(s)
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {return this;}
  virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE { return this; }

  // CefLifeSpanHandler method(s)
  virtual bool OnBeforePopup( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access ) OVERRIDE;
  virtual void OnAfterCreated( CefRefPtr<CefBrowser> browser ) OVERRIDE;
  virtual bool DoClose( CefRefPtr<CefBrowser> browser ) OVERRIDE;
  virtual void OnBeforeClose( CefRefPtr<CefBrowser> browser ) OVERRIDE;

  void runJavaScript( const std::string& js );

protected:

  virtual void showDevTools(CefRefPtr<CefBrowser> browser);
  void runJavaScriptOnUI_Thread(CefRefPtr<CefBrowser> browser, const std::string js);

  // List of open DevTools URLs if not using an external browser window.
  std::set<std::string> _openDevToolsURLs;

  // List of any browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
  BrowserList _browsers;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(Client);
  // Include the default locking implementation.
  IMPLEMENT_LOCKING(Client);
  // define logger
  DECLARE_LOGGER(Client);
};

#endif // client_h__