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

#include "include/base/cef_lock.h"
#include "include/cef_client.h"

#include <list>
#include <set>
#include <string>

class OSRBrowserProvider {
public:
  virtual CefRefPtr<CefBrowser> GetBrowser() const = 0;

protected:
  virtual ~OSRBrowserProvider() {}
};

class Client : public CefClient,
               public CefLifeSpanHandler,
               public CefKeyboardHandler,
               //public CefRenderHandler,
               public OSRBrowserProvider,
               public CefContextMenuHandler,
               public CefRequestHandler
{
public:

  // Interface implemented to handle off-screen rendering.
  /*
  class RenderHandler : public CefRenderHandler {
  public:
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) = 0;
    virtual CefWindowHandle handle() const = 0;
  };
  */

  Client();
  virtual ~Client();

  // CefClient method(s)
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {return this;}
  virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE { return this; }
  //virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE { return this; }
  virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE { return this; }
  virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }


  // CefLifeSpanHandler method(s)
  //virtual bool OnBeforePopup( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access ) OVERRIDE;
  virtual void OnAfterCreated( CefRefPtr<CefBrowser> browser ) OVERRIDE;
  virtual bool DoClose( CefRefPtr<CefBrowser> browser ) OVERRIDE;
  virtual void OnBeforeClose( CefRefPtr<CefBrowser> browser ) OVERRIDE;

  // CefRenderHandler method(s)
  /*
  virtual bool GetRootScreenRect( CefRefPtr<CefBrowser> browser, CefRect& rect ) OVERRIDE;
  virtual bool GetViewRect( CefRefPtr<CefBrowser> browser, CefRect& rect ) OVERRIDE;
  virtual bool GetScreenPoint( CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY ) OVERRIDE;
  virtual bool GetScreenInfo( CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info ) OVERRIDE;
  virtual void OnPopupShow( CefRefPtr<CefBrowser> browser, bool show ) OVERRIDE;
  virtual void OnPopupSize( CefRefPtr<CefBrowser> browser, const CefRect& rect ) OVERRIDE;
  virtual void OnPaint( CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height ) OVERRIDE;
  virtual void OnCursorChange( CefRefPtr<CefBrowser> browser, CefCursorHandle cursor ) OVERRIDE;
  */

  // CefContextMenuHandler method(s)
  virtual void OnBeforeContextMenu( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model ) OVERRIDE;
  virtual bool OnContextMenuCommand( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags ) OVERRIDE;
  virtual void OnContextMenuDismissed( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame ) OVERRIDE;

  // CefRequestHandler method(s)
  virtual bool OnQuotaRequest( CefRefPtr<CefBrowser> browser, const CefString& origin_url, int64 new_size, CefRefPtr<CefQuotaCallback> callback ) OVERRIDE;
   
  //void runJavaScript( const std::string& js );
  //void setOSRHandler(CefRefPtr<RenderHandler> handler) { _OSRHandler = handler;}
  bool IsClosing() { return _bIsClosing; }

  virtual CefRefPtr<CefBrowser> GetBrowser() const {return _browser;}

  virtual void toggleFullScreen(CefWindowHandle window) = 0;

protected:

  virtual void showDevTools(CefRefPtr<CefBrowser> browser);
  //void runJavaScriptOnUI_Thread(CefRefPtr<CefBrowser> browser, const std::string js);

  // List of open DevTools URLs if not using an external browser window.
  std::set<std::string> _openDevToolsURLs;

  // List of any browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
  BrowserList _popupBrowsers;

  //CefRefPtr<RenderHandler> _OSRHandler;

  bool _bIsClosing;

  // The child browser id
  int _browserId;
  // The child browser window
  CefRefPtr<CefBrowser> _browser;

  // Number of currently existing browser windows. The application will exit
  // when the number of windows reaches 0.
  int _browserCount;

  bool _bIsFullScreen;


  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(Client);
  // Include the default locking implementation.
  IMPLEMENT_LOCKING(Client);

};

#endif // client_h__