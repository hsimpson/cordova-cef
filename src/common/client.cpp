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

#include "include/wrapper/cef_helpers.h"

#include "include/cef_app.h"
#include "include/cef_runnable.h"
#include "humblelogging/api.h"
#include <sstream>

HUMBLE_LOGGER(logger, "Client");

Client::Client()
  : _bIsClosing(false),
    _browserId(0),
    _browserCount(0),
    _bIsFullScreen(false)
{
}

Client:: ~Client() 
{
}

/*
bool Client::OnBeforePopup( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access )
{
  if (browser->GetHost()->IsWindowRenderingDisabled()) {
    // Cancel popups in off-screen rendering mode.
    return true;
  }
  return false;
}
*/

void Client::OnAfterCreated( CefRefPtr<CefBrowser> browser )
{
  CEF_REQUIRE_UI_THREAD();
  AutoLock lock_scope(this);
  if (!_browser.get())
  {
    // We need to keep the main child window, but not popup windows
    _browser = browser;
    _browserId = browser->GetIdentifier();
  } 
  else if (browser->IsPopup())
  {
    // Add to the list of popup browsers.
    _popupBrowsers.push_back(browser);
  }

  _browserCount++;
}

bool Client::DoClose( CefRefPtr<CefBrowser> browser )
{
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (_browserId == browser->GetIdentifier())
  {
    // Set a flag to indicate that the window close should be allowed.
    _bIsClosing = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void Client::OnBeforeClose( CefRefPtr<CefBrowser> browser )
{
  CEF_REQUIRE_UI_THREAD();
  if (_browserId == browser->GetIdentifier())
  {
    // Free the browser pointer so that the browser can be destroyed
    _browser = NULL;
    /*
    if (_OSRHandler.get())
    {
      _OSRHandler->OnBeforeClose(browser);
      _OSRHandler = NULL;
    }
    */
  }
  else if (browser->IsPopup())
  {
    // Remove the record for DevTools popup windows.
    std::set<std::string>::iterator it =
      _openDevToolsURLs.find(browser->GetMainFrame()->GetURL());
    if (it != _openDevToolsURLs.end())
      _openDevToolsURLs.erase(it);

    // Remove from the browser popup list.
    BrowserList::iterator bit = _popupBrowsers.begin();
    for (; bit != _popupBrowsers.end(); ++bit)
    {
      if ((*bit)->IsSame(browser))
      {
        _popupBrowsers.erase(bit);
        break;
      }
    }
  }
  

  if (--_browserCount == 0) 
  {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void Client::showDevTools( CefRefPtr<CefBrowser> browser )
{
  std::stringstream ss;
  ss << "showDevTools, id=" << browser->GetIdentifier();
  HL_DEBUG(logger, ss.str());
  CefWindowInfo windowInfo;
  CefBrowserSettings settings;

#if defined(OS_WIN)
  windowInfo.SetAsPopup(browser->GetHost()->GetWindowHandle(), "DevTools");
#endif

  browser->GetHost()->ShowDevTools(windowInfo, this, settings, CefPoint());
}
/*
void Client::runJavaScript( const std::string& js )
{
  // ToDo: run on first or last or all browsers ??
  if(_browser.get())
  {
    if (CefCurrentlyOn(TID_UI)) 
    {
      runJavaScriptOnUI_Thread(_browser, js);
    } 
    else 
    {
      // Execute on the UI thread.
      CefPostTask(TID_UI, NewCefRunnableMethod(this, &Client::runJavaScriptOnUI_Thread, _browser, js));
    }
  }
}

void Client::runJavaScriptOnUI_Thread( CefRefPtr<CefBrowser> browser, const std::string js )
{
  CefRefPtr<CefFrame> frame = browser->GetMainFrame();
  frame->ExecuteJavaScript(js, frame->GetURL(), 0);
}
*/

/*
bool Client::GetRootScreenRect( CefRefPtr<CefBrowser> browser, CefRect& rect )
{
  if (!_OSRHandler.get())
    return false;
  return _OSRHandler->GetRootScreenRect(browser, rect);
}

bool Client::GetViewRect( CefRefPtr<CefBrowser> browser, CefRect& rect )
{
  if (!_OSRHandler.get())
    return false;
  return _OSRHandler->GetViewRect(browser, rect);
}

bool Client::GetScreenPoint( CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY )
{
  if (!_OSRHandler.get())
    return false;
  return _OSRHandler->GetScreenPoint(browser, viewX, viewY, screenX, screenY);
}

bool Client::GetScreenInfo( CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info )
{
  if (!_OSRHandler.get())
    return false;
  return _OSRHandler->GetScreenInfo(browser, screen_info);
}

void Client::OnPopupShow( CefRefPtr<CefBrowser> browser, bool show )
{
  if (!_OSRHandler.get())
    return;
  return _OSRHandler->OnPopupShow(browser, show);
}

void Client::OnPopupSize( CefRefPtr<CefBrowser> browser, const CefRect& rect )
{
  if (!_OSRHandler.get())
    return;
  return _OSRHandler->OnPopupSize(browser, rect);
}

void Client::OnPaint( CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height )
{
  if (!_OSRHandler.get())
    return;
  return _OSRHandler->OnPaint(browser, type, dirtyRects, buffer, width, height);
}

void Client::OnCursorChange( CefRefPtr<CefBrowser> browser, CefCursorHandle cursor )
{
  if (!_OSRHandler.get())
    return;
  return _OSRHandler->OnCursorChange(browser, cursor);
}
*/

void Client::OnBeforeContextMenu( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model )
{
#ifndef _DEBUG
  model->Clear();
#endif
}

bool Client::OnContextMenuCommand( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags )
{
  return false;
}

void Client::OnContextMenuDismissed( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame )
{
}

bool Client::OnQuotaRequest( CefRefPtr<CefBrowser> browser, const CefString& origin_url, int64 new_size, CefRefPtr<CefQuotaCallback> callback )
{
  callback->Continue(true);
  return true;
}

