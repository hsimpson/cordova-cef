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

#ifndef osrwindow_win_h__
#define osrwindow_win_h__

#include "common/client.h"
#include "cefclient/osrenderer.h"

class OSRWindow_Win : public Client::RenderHandler
{
public:
  OSRWindow_Win(CefWindowHandle parent, OSRBrowserProvider* browser_provider, bool transparent);
  virtual ~OSRWindow_Win();

  virtual CefWindowHandle handle() const {return _hWnd;}

  virtual void OnBeforeClose( CefRefPtr<CefBrowser> browser ); 
  virtual bool GetRootScreenRect( CefRefPtr<CefBrowser> browser, CefRect& rect );
  virtual bool GetViewRect( CefRefPtr<CefBrowser> browser, CefRect& rect );
  virtual bool GetScreenPoint( CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY );
  //virtual bool GetScreenInfo( CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info );
  virtual void OnPopupShow( CefRefPtr<CefBrowser> browser, bool show );
  virtual void OnPopupSize( CefRefPtr<CefBrowser> browser, const CefRect& rect );
  virtual void OnPaint( CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height );
  virtual void OnCursorChange( CefRefPtr<CefBrowser> browser, CefCursorHandle cursor );

  void Invalidate();

private:
  // Create the underlying window.
  bool CreateWidget(HWND hWndParent, const RECT& rect, HINSTANCE hInst, LPCTSTR className);

  // Destroy the underlying window.
  void DestroyWidget();

  void Render();
  void EnableGL();
  void DisableGL();
  void OnDestroyed();
  static ATOM RegisterOSRClass(HINSTANCE hInstance, LPCTSTR className);
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
    LPARAM lParam);
  static int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam);
  static int GetCefMouseModifiers(WPARAM wparam);
  static bool isKeyDown(WPARAM wparam);
  bool IsOverPopupWidget(int x, int y) const;
  int GetPopupXOffset() const;
  int GetPopupYOffset() const;
  void ApplyPopupOffset(int& x, int& y) const;

  ClientOSRenderer _renderer;
  OSRBrowserProvider* _browser_provider;
  HWND _hWnd;
  HDC _hDC;
  HGLRC _hRC;

  bool _painting_popup;
  bool _render_task_pending;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(OSRWindow_Win);
};

#endif // osrwindow_win_h__