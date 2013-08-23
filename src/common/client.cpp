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

int Client::_browserCount = 0;

Client::Client() 
  : _browserId(0),
    _bIsClosing(false)
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
  if (!_browser.get())   {
    // We need to keep the main child window, but not popup windows
    _browser = browser;
    _browserId = browser->GetIdentifier();
  } else if (browser->IsPopup()) {
    // Add to the list of popup browsers.
    _popupBrowsers.push_back(browser);
  }

  _browserCount++;
}

bool Client::DoClose( CefRefPtr<CefBrowser> browser )
{
  REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed description of this
  // process.
  if (_browserId == browser->GetIdentifier()) {
    // Notify the browser that the parent window is about to close.
    browser->GetHost()->ParentWindowWillClose();

    // Set a flag to indicate that the window close should be allowed.
    _bIsClosing = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void Client::OnBeforeClose( CefRefPtr<CefBrowser> browser )
{
  REQUIRE_UI_THREAD();

  if (_browserId == browser->GetIdentifier()) {
    // Free the browser pointer so that the browser can be destroyed
    _browser = NULL;

    /*
    if (m_OSRHandler.get()) {
      m_OSRHandler->OnBeforeClose(browser);
      m_OSRHandler = NULL;
    }
    */
  } else if (browser->IsPopup()) {
    // Remove the record for DevTools popup windows.
    
    std::set<std::string>::iterator it =
      _openDevToolsURLs.find(browser->GetMainFrame()->GetURL());
    if (it != _openDevToolsURLs.end())
      _openDevToolsURLs.erase(it);
    

    // Remove from the browser popup list.
    BrowserList::iterator bit = _popupBrowsers.begin();
    for (; bit != _popupBrowsers.end(); ++bit) {
      if ((*bit)->IsSame(browser)) {
        _popupBrowsers.erase(bit);
        break;
      }
    }
  }

  if (--_browserCount == 0) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void Client::showDevTools( CefRefPtr<CefBrowser> browser )
{
  std::string devtools_url = browser->GetHost()->GetDevToolsURL(true);
  if(!devtools_url.empty())
  {
    if(_openDevToolsURLs.find(devtools_url) == _openDevToolsURLs.end())
    {
      // Open DevTools in a popup window.
      _openDevToolsURLs.insert(devtools_url);
      browser->GetMainFrame()->ExecuteJavaScript(
        "window.open('" +  devtools_url + "');", "about:blank", 0);
    }
  }
}