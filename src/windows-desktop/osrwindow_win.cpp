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

#include "osrwindow_win.h"
#include <windowsx.h>
#include "include/cef_runnable.h"
#include "cefclient/util.h"
#include <gl/gl.h>


OSRWindow_Win::OSRWindow_Win(CefWindowHandle parent, OSRBrowserProvider* browser_provider, bool transparent)
  : _renderer(transparent),
    _browser_provider(browser_provider),
    _hWnd(NULL),
    _hDC(NULL),
    _hRC(NULL),
    _painting_popup(false),
    _render_task_pending(false)
{
  RECT r;
  r.left = 0;
  r.top = 0;
  r.right = 800;
  r.bottom = 600;
  CreateWidget(parent, r, GetModuleHandle(NULL), L"CORDOVA-CEF-OSR-WIDGET");
}

OSRWindow_Win::~OSRWindow_Win()
{
  DestroyWidget();
}

void OSRWindow_Win::OnBeforeClose( CefRefPtr<CefBrowser> browser )
{
  DisableGL();
}

bool OSRWindow_Win::GetRootScreenRect( CefRefPtr<CefBrowser> browser, CefRect& rect )
{
  RECT window_rect = {0};
  HWND root_window = GetAncestor(_hWnd, GA_ROOT);
  if (::GetWindowRect(root_window, &window_rect)) {
    rect = CefRect(window_rect.left,
      window_rect.top,
      window_rect.right - window_rect.left,
      window_rect.bottom - window_rect.top);
    return true;
  }
  return false;
}

bool OSRWindow_Win::GetViewRect( CefRefPtr<CefBrowser> browser, CefRect& rect )
{
  RECT clientRect;
  if (!::GetClientRect(_hWnd, &clientRect))
    return false;
  rect.x = rect.y = 0;
  rect.width = clientRect.right;
  rect.height = clientRect.bottom;
  return true;
}

bool OSRWindow_Win::GetScreenPoint( CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY )
{
  if (!::IsWindow(_hWnd))
    return false;

  // Convert the point from view coordinates to actual screen coordinates.
  POINT screen_pt = {viewX, viewY};
  ClientToScreen(_hWnd, &screen_pt);
  screenX = screen_pt.x;
  screenY = screen_pt.y;
  return true;
}

/*
bool OSRWindow_Win::GetScreenInfo( CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info )
{
  throw std::logic_error("The method or operation is not implemented.");
}
*/

void OSRWindow_Win::OnPopupShow( CefRefPtr<CefBrowser> browser, bool show )
{
  if (!show) {
    CefRect dirty_rect = _renderer.popup_rect();
    _renderer.ClearPopupRects();
    browser->GetHost()->Invalidate(dirty_rect, PET_VIEW);
  }
  _renderer.OnPopupShow(browser, show);
}

void OSRWindow_Win::OnPopupSize( CefRefPtr<CefBrowser> browser, const CefRect& rect )
{
  _renderer.OnPopupSize(browser, rect);
}

void OSRWindow_Win::OnPaint( CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height )
{
  if (_painting_popup) {
    _renderer.OnPaint(browser, type, dirtyRects, buffer, width, height);
    return;
  }
  if (!_hDC)
    EnableGL();

  wglMakeCurrent(_hDC, _hRC);
  _renderer.OnPaint(browser, type, dirtyRects, buffer, width, height);
  if (type == PET_VIEW && !_renderer.popup_rect().IsEmpty()) {
    _painting_popup = true;
    CefRect client_popup_rect(0, 0,
      _renderer.popup_rect().width,
      _renderer.popup_rect().height);
    browser->GetHost()->Invalidate(client_popup_rect, PET_POPUP);
    _painting_popup = false;
  }
  _renderer.Render();
  SwapBuffers(_hDC);
}

void OSRWindow_Win::OnCursorChange( CefRefPtr<CefBrowser> browser, CefCursorHandle cursor )
{
  if (!::IsWindow(_hWnd))
    return;

  // Change the plugin window's cursor.
  SetClassLongW(_hWnd, GCL_HCURSOR,
    static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
  SetCursor(cursor);
}

bool OSRWindow_Win::CreateWidget( HWND hWndParent, const RECT& rect, HINSTANCE hInst, LPCTSTR className )
{
  ASSERT(_hWnd == NULL && _hDC == NULL && _hRC == NULL);

  RegisterOSRClass(hInst, className);
  _hWnd = ::CreateWindowW(className, 0,
    WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
    rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
   hWndParent, 0, hInst, 0);

  if (!_hWnd)
    return false;

  SetWindowLongPtrW(_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  //SetWindowLongW(_hWnd, GWL_EXSTYLE, GetWindowLongW(_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  // Make red pixels transparent:
  //SetLayeredWindowAttributes(_hWnd, 0, 255, LWA_ALPHA);

  // Reference released in OnDestroyed().
  AddRef();

  return true;
}

void OSRWindow_Win::DestroyWidget()
{
  if (IsWindow(_hWnd))
    DestroyWindow(_hWnd);
}

void OSRWindow_Win::OnDestroyed()
{
  SetWindowLongPtrW(_hWnd, GWLP_USERDATA, 0L);
  _hWnd = NULL;
  Release();
}

ATOM OSRWindow_Win::RegisterOSRClass( HINSTANCE hInstance, LPCTSTR className )
{
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style         = CS_OWNDC;
  wcex.lpfnWndProc   = &OSRWindow_Win::WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = hInstance;
  wcex.hIcon         = NULL;
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = className;
  wcex.hIconSm       = NULL;
  return RegisterClassExW(&wcex);
}

void OSRWindow_Win::Invalidate()
{
  if (!CefCurrentlyOn(TID_UI)) {
    CefPostTask(TID_UI, NewCefRunnableMethod(this, &OSRWindow_Win::Invalidate));
    return;
  }

  // Don't post another task if the previous task is still pending.
  if (_render_task_pending)
    return;

  _render_task_pending = true;

  // Render at 30fps.
  static const int kRenderDelay = 1 / 30;
  CefPostDelayedTask(TID_UI, NewCefRunnableMethod(this, &OSRWindow_Win::Render),
    kRenderDelay);
}

void OSRWindow_Win::Render()
{
  ASSERT(CefCurrentlyOn(TID_UI));
  if (_render_task_pending)
    _render_task_pending = false;

  if (!_hDC)
    EnableGL();

  wglMakeCurrent(_hDC, _hRC);
  _renderer.Render();
  SwapBuffers(_hDC);
}

void OSRWindow_Win::EnableGL()
{
  ASSERT(CefCurrentlyOn(TID_UI));

  //PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

 

  // Get the device context.
  _hDC = GetDC(_hWnd);

  PIXELFORMATDESCRIPTOR pfd = 
  {
    sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
    1,                              // version number
    PFD_DRAW_TO_WINDOW |            // support window
    PFD_SUPPORT_OPENGL |            // support OpenGL
    PFD_DOUBLEBUFFER,               // double buffered
    PFD_TYPE_RGBA,                  // RGBA type
    32,                             // 32-bit color depth
    0, 0, 0, 0, 0, 0,               // color bits ignored
    8,                              // no alpha buffer
    0,                              // shift bit ignored
    0,                              // no accumulation buffer
    0, 0, 0, 0,                     // accumulation bits ignored
    32,                             // 32-bit z-buffer
    0,                              // no stencil buffer
    0,                              // no auxiliary buffer
    PFD_MAIN_PLANE,                 // main layer
    0,                              // reserved
    0, 0, 0                         // layer masks ignored
  };

  int format = ChoosePixelFormat(_hDC, &pfd);
  SetPixelFormat(_hDC, format, &pfd);

  PIXELFORMATDESCRIPTOR pfd_new;
  DescribePixelFormat(_hDC, format, sizeof(PIXELFORMATDESCRIPTOR), &pfd_new);

  // Create and enable the render context.
  _hRC = wglCreateContext(_hDC);
  wglMakeCurrent(_hDC, _hRC);

  const char* glversion = (const char*)glGetString(GL_VERSION);

  _renderer.Initialize();
}

void OSRWindow_Win::DisableGL()
{
  ASSERT(CefCurrentlyOn(TID_UI));

  if (!_hDC)
    return;

  _renderer.Cleanup();

  if (IsWindow(_hWnd)) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(_hRC);
    ReleaseDC(_hWnd, _hDC);
  }

  _hDC = NULL;
  _hRC = NULL;
}


int OSRWindow_Win::GetCefKeyboardModifiers( WPARAM wparam, LPARAM lparam )
{
  int modifiers = 0;
  if (isKeyDown(VK_SHIFT))
    modifiers |= EVENTFLAG_SHIFT_DOWN;
  if (isKeyDown(VK_CONTROL))
    modifiers |= EVENTFLAG_CONTROL_DOWN;
  if (isKeyDown(VK_MENU))
    modifiers |= EVENTFLAG_ALT_DOWN;

  // Low bit set from GetKeyState indicates "toggled".
  if (::GetKeyState(VK_NUMLOCK) & 1)
    modifiers |= EVENTFLAG_NUM_LOCK_ON;
  if (::GetKeyState(VK_CAPITAL) & 1)
    modifiers |= EVENTFLAG_CAPS_LOCK_ON;

  switch (wparam) {
  case VK_RETURN:
    if ((lparam >> 16) & KF_EXTENDED)
      modifiers |= EVENTFLAG_IS_KEY_PAD;
    break;
  case VK_INSERT:
  case VK_DELETE:
  case VK_HOME:
  case VK_END:
  case VK_PRIOR:
  case VK_NEXT:
  case VK_UP:
  case VK_DOWN:
  case VK_LEFT:
  case VK_RIGHT:
    if (!((lparam >> 16) & KF_EXTENDED))
      modifiers |= EVENTFLAG_IS_KEY_PAD;
    break;
  case VK_NUMLOCK:
  case VK_NUMPAD0:
  case VK_NUMPAD1:
  case VK_NUMPAD2:
  case VK_NUMPAD3:
  case VK_NUMPAD4:
  case VK_NUMPAD5:
  case VK_NUMPAD6:
  case VK_NUMPAD7:
  case VK_NUMPAD8:
  case VK_NUMPAD9:
  case VK_DIVIDE:
  case VK_MULTIPLY:
  case VK_SUBTRACT:
  case VK_ADD:
  case VK_DECIMAL:
  case VK_CLEAR:
    modifiers |= EVENTFLAG_IS_KEY_PAD;
    break;
  case VK_SHIFT:
    if (isKeyDown(VK_LSHIFT))
      modifiers |= EVENTFLAG_IS_LEFT;
    else if (isKeyDown(VK_RSHIFT))
      modifiers |= EVENTFLAG_IS_RIGHT;
    break;
  case VK_CONTROL:
    if (isKeyDown(VK_LCONTROL))
      modifiers |= EVENTFLAG_IS_LEFT;
    else if (isKeyDown(VK_RCONTROL))
      modifiers |= EVENTFLAG_IS_RIGHT;
    break;
  case VK_MENU:
    if (isKeyDown(VK_LMENU))
      modifiers |= EVENTFLAG_IS_LEFT;
    else if (isKeyDown(VK_RMENU))
      modifiers |= EVENTFLAG_IS_RIGHT;
    break;
  case VK_LWIN:
    modifiers |= EVENTFLAG_IS_LEFT;
    break;
  case VK_RWIN:
    modifiers |= EVENTFLAG_IS_RIGHT;
    break;
  }
  return modifiers;
}

int OSRWindow_Win::GetCefMouseModifiers( WPARAM wparam )
{
  int modifiers = 0;
  if (wparam & MK_CONTROL)
    modifiers |= EVENTFLAG_CONTROL_DOWN;
  if (wparam & MK_SHIFT)
    modifiers |= EVENTFLAG_SHIFT_DOWN;
  if (isKeyDown(VK_MENU))
    modifiers |= EVENTFLAG_ALT_DOWN;
  if (wparam & MK_LBUTTON)
    modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
  if (wparam & MK_MBUTTON)
    modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
  if (wparam & MK_RBUTTON)
    modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;

  // Low bit set from GetKeyState indicates "toggled".
  if (::GetKeyState(VK_NUMLOCK) & 1)
    modifiers |= EVENTFLAG_NUM_LOCK_ON;
  if (::GetKeyState(VK_CAPITAL) & 1)
    modifiers |= EVENTFLAG_CAPS_LOCK_ON;
  return modifiers;
}

bool OSRWindow_Win::isKeyDown( WPARAM wparam )
{
  return (GetKeyState(wparam) & 0x8000) != 0;
}

bool OSRWindow_Win::IsOverPopupWidget( int x, int y ) const
{
  const CefRect& rc = _renderer.popup_rect();
  int popup_right = rc.x + rc.width;
  int popup_bottom = rc.y + rc.height;
  return (x >= rc.x) && (x < popup_right) &&
    (y >= rc.y) && (y < popup_bottom);
}

int OSRWindow_Win::GetPopupXOffset() const
{
  return _renderer.original_popup_rect().x - _renderer.popup_rect().x;
}

int OSRWindow_Win::GetPopupYOffset() const
{
  return _renderer.original_popup_rect().y - _renderer.popup_rect().y;
}

void OSRWindow_Win::ApplyPopupOffset( int& x, int& y ) const
{
  if (IsOverPopupWidget(x, y)) {
    x += GetPopupXOffset();
    y += GetPopupYOffset();
  }
}

LRESULT CALLBACK OSRWindow_Win::WndProc(HWND hWnd, UINT message,
                                    WPARAM wParam, LPARAM lParam) {
  static POINT lastMousePos, curMousePos;
  static bool mouseRotation = false;
  static bool mouseTracking = false;

  static int lastClickX = 0;
  static int lastClickY = 0;
  static CefBrowserHost::MouseButtonType lastClickButton = MBT_LEFT;
  static int gLastClickCount = 0;
  static double gLastClickTime = 0;

  static bool gLastMouseDownOnView = false;

  OSRWindow_Win* window =
      reinterpret_cast<OSRWindow_Win*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
  CefRefPtr<CefBrowserHost> browser;
  if (window && window->_browser_provider->GetBrowser().get())
    browser = window->_browser_provider->GetBrowser()->GetHost();

  LONG currentTime = 0;
  bool cancelPreviousClick = false;

  if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
      message == WM_MBUTTONDOWN || message == WM_MOUSEMOVE ||
      message == WM_MOUSELEAVE) {
    currentTime = GetMessageTime();
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    cancelPreviousClick =
        (abs(lastClickX - x) > (GetSystemMetrics(SM_CXDOUBLECLK) / 2))
        || (abs(lastClickY - y) > (GetSystemMetrics(SM_CYDOUBLECLK) / 2))
        || ((currentTime - gLastClickTime) > GetDoubleClickTime());
    if (cancelPreviousClick &&
        (message == WM_MOUSEMOVE || message == WM_MOUSELEAVE)) {
      gLastClickCount = 0;
      lastClickX = 0;
      lastClickY = 0;
      gLastClickTime = 0;
    }
  }

  switch (message) {
  case WM_DESTROY:
    if (window)
      window->OnDestroyed();
    return 0;

  case WM_LBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MBUTTONDOWN: {
    SetCapture(hWnd);
    SetFocus(hWnd);
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    if (wParam & MK_SHIFT) {
      // Start rotation effect.
      lastMousePos.x = curMousePos.x = x;
      lastMousePos.y = curMousePos.y = y;
      mouseRotation = true;
    } else {
      CefBrowserHost::MouseButtonType btnType =
          (message == WM_LBUTTONDOWN ? MBT_LEFT : (
           message == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
      if (!cancelPreviousClick && (btnType == lastClickButton)) {
        ++gLastClickCount;
      } else {
        gLastClickCount = 1;
        lastClickX = x;
        lastClickY = y;
      }
      gLastClickTime = currentTime;
      lastClickButton = btnType;

      if (browser.get()) {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        gLastMouseDownOnView = !window->IsOverPopupWidget(x, y);
        window->ApplyPopupOffset(mouse_event.x, mouse_event.y);
        mouse_event.modifiers = GetCefMouseModifiers(wParam);
        browser->SendMouseClickEvent(mouse_event, btnType, false,
                                     gLastClickCount);
      }
    }
    break;
  }

  case WM_LBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MBUTTONUP:
    if (GetCapture() == hWnd)
      ReleaseCapture();
    if (mouseRotation) {
      // End rotation effect.
      mouseRotation = false;
      window->_renderer.SetSpin(0, 0);
      window->Invalidate();
    } else {
      int x = GET_X_LPARAM(lParam);
      int y = GET_Y_LPARAM(lParam);
      CefBrowserHost::MouseButtonType btnType =
          (message == WM_LBUTTONUP ? MBT_LEFT : (
           message == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));
      if (browser.get()) {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        if (gLastMouseDownOnView &&
            window->IsOverPopupWidget(x, y) &&
            (window->GetPopupXOffset() || window->GetPopupYOffset())) {
          break;
        }
        window->ApplyPopupOffset(mouse_event.x, mouse_event.y);
        mouse_event.modifiers = GetCefMouseModifiers(wParam);
        browser->SendMouseClickEvent(mouse_event, btnType, true,
                                     gLastClickCount);
      }
    }
    break;

  case WM_MOUSEMOVE: {
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    if (mouseRotation) {
      // Apply rotation effect.
      curMousePos.x = x;
      curMousePos.y = y;
      window->_renderer.IncrementSpin((curMousePos.x - lastMousePos.x),
        (curMousePos.y - lastMousePos.y));
      lastMousePos.x = curMousePos.x;
      lastMousePos.y = curMousePos.y;
      window->Invalidate();
    } else {
      if (!mouseTracking) {
        // Start tracking mouse leave. Required for the WM_MOUSELEAVE event to
        // be generated.
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hWnd;
        TrackMouseEvent(&tme);
        mouseTracking = true;
      }
      if (browser.get()) {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        window->ApplyPopupOffset(mouse_event.x, mouse_event.y);
        mouse_event.modifiers = GetCefMouseModifiers(wParam);
        browser->SendMouseMoveEvent(mouse_event, false);
      }
    }
    break;
  }

  case WM_MOUSELEAVE:
    if (mouseTracking) {
      // Stop tracking mouse leave.
      TRACKMOUSEEVENT tme;
      tme.cbSize = sizeof(TRACKMOUSEEVENT);
      tme.dwFlags = TME_LEAVE & TME_CANCEL;
      tme.hwndTrack = hWnd;
      TrackMouseEvent(&tme);
      mouseTracking = false;
    }
    if (browser.get()) {
      CefMouseEvent mouse_event;
      mouse_event.x = 0;
      mouse_event.y = 0;
      mouse_event.modifiers = GetCefMouseModifiers(wParam);
      browser->SendMouseMoveEvent(mouse_event, true);
    }
    break;

  case WM_MOUSEWHEEL:
    if (browser.get()) {
      POINT screen_point = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      HWND scrolled_wnd = ::WindowFromPoint(screen_point);
      if (scrolled_wnd != hWnd) {
        break;
      }
      ScreenToClient(hWnd, &screen_point);
      int delta = GET_WHEEL_DELTA_WPARAM(wParam);

      CefMouseEvent mouse_event;
      mouse_event.x = screen_point.x;
      mouse_event.y = screen_point.y;
      window->ApplyPopupOffset(mouse_event.x, mouse_event.y);
      mouse_event.modifiers = GetCefMouseModifiers(wParam);

      browser->SendMouseWheelEvent(mouse_event,
                                   isKeyDown(VK_SHIFT) ? delta : 0,
                                   !isKeyDown(VK_SHIFT) ? delta : 0);
    }
    break;

  case WM_SIZE:
    if (browser.get())
      browser->WasResized();
    break;

  case WM_SETFOCUS:
  case WM_KILLFOCUS:
    if (browser.get())
      browser->SendFocusEvent(message == WM_SETFOCUS);
    break;

  case WM_CAPTURECHANGED:
  case WM_CANCELMODE:
    if (!mouseRotation) {
      if (browser.get())
        browser->SendCaptureLostEvent();
    }
    break;
  case WM_SYSCHAR:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_CHAR: {
    CefKeyEvent event;
    event.windows_key_code = wParam;
    event.native_key_code = lParam;
    event.is_system_key = message == WM_SYSCHAR ||
                          message == WM_SYSKEYDOWN ||
                          message == WM_SYSKEYUP;

    if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
      event.type = KEYEVENT_RAWKEYDOWN;
    else if (message == WM_KEYUP || message == WM_SYSKEYUP)
      event.type = KEYEVENT_KEYUP;
    else
      event.type = KEYEVENT_CHAR;
    event.modifiers = GetCefKeyboardModifiers(wParam, lParam);
    if (browser.get())
      browser->SendKeyEvent(event);
    break;
  }

  case WM_PAINT: {
    PAINTSTRUCT ps;
    RECT rc;
    BeginPaint(hWnd, &ps);
    rc = ps.rcPaint;
    EndPaint(hWnd, &ps);
    if (browser.get()) {
      browser->Invalidate(CefRect(rc.left,
                                  rc.top,
                                  rc.right - rc.left,
                                  rc.bottom - rc.top), PET_VIEW);
    }
    return 0;
  }

  case WM_ERASEBKGND:
    return 0;
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}

