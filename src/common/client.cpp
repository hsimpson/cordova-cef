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

#include "client.h"
#include "util.h"

#include "include/cef_app.h"
#include "include/cef_runnable.h"
#include "logging.h"


Client::Client()
  :INIT_LOGGER(Client)
{
}

Client:: ~Client() 
{
}

bool Client::OnBeforePopup( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access )
{
  if (browser->GetHost()->IsWindowRenderingDisabled()) {
    // Cancel popups in off-screen rendering mode.
    return true;
  }
  return false;
}

void Client::OnAfterCreated( CefRefPtr<CefBrowser> browser )
{
  REQUIRE_UI_THREAD();
  AutoLock lock_scope(this);
  // Add to the list of browsers.
  _browsers.push_back(browser);
}

bool Client::DoClose( CefRefPtr<CefBrowser> browser )
{
  REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed description of this
  // process.
  if (_browsers.size() == 1 && _browsers.front()->IsSame(browser)) {
    // Notify the browser that the parent window is about to close.
    browser->GetHost()->ParentWindowWillClose();
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void Client::OnBeforeClose( CefRefPtr<CefBrowser> browser )
{
  REQUIRE_UI_THREAD();
    
  std::set<std::string>::iterator it =
    _openDevToolsURLs.find(browser->GetMainFrame()->GetURL());
  if (it != _openDevToolsURLs.end())
    _openDevToolsURLs.erase(it);
    

  // Remove from the browser popup list.
  BrowserList::iterator bit = _browsers.begin();
  for (; bit != _browsers.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      _browsers.erase(bit);
      break;
    }
  }
  

  if (_browsers.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void Client::showDevTools( CefRefPtr<CefBrowser> browser )
{
  LOG_DEBUG(logger()) << "showDevTools, id=" << browser->GetIdentifier();

  std::string devtools_url = browser->GetHost()->GetDevToolsURL(true);
  if(!devtools_url.empty())
  {
    if(_openDevToolsURLs.find(devtools_url) == _openDevToolsURLs.end())
    {
      // Open DevTools in a popup window.
      _openDevToolsURLs.insert(devtools_url);

      /*
      browser->GetMainFrame()->ExecuteJavaScript(
        "window.open('" +  devtools_url + "');", "about:blank", 0);
      */

      CefWindowInfo info;
      info.SetAsPopup(NULL, devtools_url);

      CefBrowserSettings browserSettings;
      //browserSettings.developer_tools = STATE_DISABLED;
      browserSettings.file_access_from_file_urls = STATE_ENABLED;
      browserSettings.universal_access_from_file_urls = STATE_ENABLED;
      browserSettings.web_security = STATE_DISABLED;

      CefBrowserHost::CreateBrowser(info, this, devtools_url, browserSettings);
    }
  }
}

void Client::runJavaScript( const std::string& js )
{
  // ToDo: run on first or last or all browsers ??
  if(!_browsers.empty())
  {
    CefRefPtr<CefBrowser> browser = _browsers.back();
    if (CefCurrentlyOn(TID_UI)) 
    {
      runJavaScriptOnUI_Thread(browser, js);
    } 
    else 
    {
      // Execute on the UI thread.
      CefPostTask(TID_UI, NewCefRunnableMethod(this, &Client::runJavaScriptOnUI_Thread, browser, js));
    }
  }
}

void Client::runJavaScriptOnUI_Thread( CefRefPtr<CefBrowser> browser, const std::string js )
{
  CefRefPtr<CefFrame> frame = browser->GetMainFrame();
  frame->ExecuteJavaScript(js, frame->GetURL(), 0);
}
