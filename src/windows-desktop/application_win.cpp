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

#include "application_win.h"
#include "client_win.h"
#include "osrwindow_win.h"
#include "resource.h"

#include <Shlwapi.h>
#include <algorithm>

Application_Win::Application_Win(std::shared_ptr<Helper::Paths> paths)
  : INIT_LOGGER(Application_Win),
    Application(new Client_Win, paths)
{
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style         = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc   = &Application_Win::WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = GetModuleHandle(NULL);
  wcex.hIcon         = NULL;
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  HBITMAP hbmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDB_BITMAP_CHECKERBOARD));
  wcex.hbrBackground = CreatePatternBrush(hbmp);
  
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = L"CORDOVA-CEF-MAINWINDOW";
  wcex.hIconSm       = NULL;

  RegisterClassExW(&wcex);

  DWORD window_style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;

  RECT neededRect;
  // TODO: move to config.xml
  neededRect.left = 0;
  neededRect.top = 0;
  neededRect.right = 800;
  neededRect.bottom = 600;

  AdjustWindowRect(&neededRect, window_style, false);

  int offset_x = 0 - neededRect.left;
  int offset_y = 0 - neededRect.top;

  int nWidth = neededRect.right - neededRect.left;
  int nHeight = neededRect.bottom - neededRect.top;

  int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
  int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

  offset_x += ((nScreenWidth / 2) - (nWidth / 2));
  offset_y += ((nScreenHeight / 2) - (nHeight / 2));

  neededRect.left   += offset_x;
  neededRect.top    += offset_y;
  neededRect.right  += offset_x;
  neededRect.bottom += offset_y;

  _mainWindow = CreateWindowW(L"CORDOVA-CEF-MAINWINDOW", Helper::utf8ToWide(_config->appName()).c_str(),
                              window_style, 
                              neededRect.left, 
                              neededRect.top, 
                              neededRect.right - neededRect.left,
                              neededRect.bottom - neededRect.top, 
                              NULL, NULL, GetModuleHandle(NULL), NULL);

  SetWindowLongPtrW(_mainWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
 
  ShowWindow(_mainWindow, SW_SHOW );
  UpdateWindow(_mainWindow);
}

Application_Win::~Application_Win()
{
}

CefRefPtr<Client::RenderHandler> Application_Win::createOSRWindow(CefWindowHandle parent, OSRBrowserProvider* browser_provider, bool transparent)
{
  return new OSRWindow_Win(parent, browser_provider, transparent);
}

LRESULT CALLBACK Application_Win::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  Application_Win* appwindow = reinterpret_cast<Application_Win*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
  PAINTSTRUCT ps;
  HDC hdc;
  switch (message) 
  {
    case WM_PAINT:
    {
      hdc = BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
      return 0;
    }

    case WM_KILLFOCUS:
    {
      appwindow->handlePause();
      break;
    }

    case WM_SETFOCUS:
      appwindow->handleResume();
      if (appwindow->_client.get() && appwindow->_client->GetBrowser()) 
      {
        // Pass focus to the browser window
        CefWindowHandle hwnd = appwindow->_client->GetBrowser()->GetHost()->GetWindowHandle();
        if (hwnd)
          PostMessage(hwnd, WM_SETFOCUS, wParam, NULL);
      }
      return 0;

    case WM_SIZE:
    // Minimizing resizes the window to 0x0 which causes our layout to go all
    // screwy, so we just ignore it.
    if (wParam != SIZE_MINIMIZED && appwindow->_client.get() && appwindow->_client->GetBrowser()) 
    {
        CefWindowHandle hwnd = appwindow->_client->GetBrowser()->GetHost()->GetWindowHandle();
        if (hwnd) 
        {
          // Resize the browser window and address bar to match the new frame
          // window size
          RECT rect;
          GetClientRect(hWnd, &rect);
          
          HDWP hdwp = BeginDeferWindowPos(1);
          hdwp = DeferWindowPos(hdwp, hwnd, NULL,
            rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
          EndDeferWindowPos(hdwp);
        }
    }
    break;



    case WM_CLOSE:
    {
      if (appwindow->_client.get() && !appwindow->_client->IsClosing()) {
        CefRefPtr<CefBrowser> browser = appwindow->_client->GetBrowser();
        if (browser.get()) 
        {
          // Notify the browser window that we would like to close it. This
          // will result in a call to ClientHandler::DoClose() if the
          // JavaScript 'onbeforeunload' event handler allows it.
          browser->GetHost()->CloseBrowser(false);

          // Cancel the close.
          return 0;
        }
      }

      // Allow the close.
      break;
    }   

    case WM_DESTROY:
    {
      // Quitting CEF is handled in ClientHandler::OnBeforeClose().
      return 0;
    }
  }
  return DefWindowProcW(hWnd, message, wParam, lParam);
}

