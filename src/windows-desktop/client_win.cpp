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

#include <sstream>
#include "client_win.h"
#include "include/wrapper/cef_helpers.h"
#include "humblelogging/api.h"
#include <Winuser.h>


HUMBLE_LOGGER(logger, "client");

Client_Win::Client_Win()
  : Client()
{

}

Client_Win::~Client_Win()
{

}

bool Client_Win::OnKeyEvent( CefRefPtr<CefBrowser> browser, const CefKeyEvent &event, CefEventHandle os_event )
{
  // see chrome shortcuts: https://support.google.com/chrome/topic/25799?hl=en
  // see windows key codes: http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
  CEF_REQUIRE_UI_THREAD();


  if(HL_IF_TRACE(logger))
  {
    std::stringstream sstream;
    sstream << "OnKeyEvent, type=" << event.type
            << " modifiers=" << event.modifiers
            << " windows_key_code=" << event.windows_key_code
            << " unmodified_character=" << event.unmodified_character;

    HL_TRACE(logger, sstream.str());
  }


  if(!event.focus_on_editable_field && event.type == KEYEVENT_RAWKEYDOWN) // shortcut only on non edit field
  {
    if(event.modifiers == EVENTFLAG_NONE) // modifiers: none
    {
      switch (event.windows_key_code)
      {
        case VK_F5:
        {
          browser->Reload();
          return true;
        }
        case  VK_F11:
        {
          toggleFullScreen(browser->GetHost()->GetWindowHandle());
          return true;
        }
        default:
          break;
      }
    }
    if(event.modifiers & EVENTFLAG_CONTROL_DOWN) // modifiers: CTRL
    {
      switch (event.windows_key_code)
      {
        case VK_F5:
        case 0x52: // R - Key
        {
          browser->ReloadIgnoreCache();
          return true;
        }
        default:
          break;
      }
    }
    if(event.modifiers & EVENTFLAG_SHIFT_DOWN) // modifiers: SHIFT
    {
      switch (event.windows_key_code)
      {
        case VK_F5:
        {
          browser->ReloadIgnoreCache();
          return true;
        }
        default:
          break;
      }
    }
    if( (event.modifiers & EVENTFLAG_SHIFT_DOWN) && (event.modifiers & EVENTFLAG_CONTROL_DOWN) ) // modifiers: SHIFT + CTRL
    {
      switch (event.windows_key_code)
      {
        case 0x49: // I - Key
        {
          showDevTools(browser);
          return true;
        }
        default:
          break;
      }
    }
  }


  return false;
}

void Client_Win::toggleFullScreen(CefWindowHandle window)
{
  window = GetParent(window);
  // Save current window state if not already fullscreen.
  if(!_bIsFullScreen)
  {
    _savedWindowInfo.maximized = ::IsZoomed(window);
    if(_savedWindowInfo.maximized)
      ::SendMessage(window, WM_SYSCOMMAND, SC_RESTORE, 0);
    _savedWindowInfo.style = GetWindowLong(window, GWL_STYLE);
    _savedWindowInfo.ex_style = GetWindowLong(window, GWL_EXSTYLE);
    GetWindowRect(window, &_savedWindowInfo.rect);
  }

  _bIsFullScreen = ! _bIsFullScreen; // toggle
  if(_bIsFullScreen)
  {
    SetWindowLong(window, GWL_STYLE,
                  _savedWindowInfo.style & ~(WS_CAPTION | WS_THICKFRAME));
    SetWindowLong(window, GWL_EXSTYLE,
                  _savedWindowInfo.ex_style & ~(WS_EX_DLGMODALFRAME |
                      WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(monitor_info);
    GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST),
                   &monitor_info);
    SetWindowPos(window, NULL,
                 monitor_info.rcMonitor.left,
                 monitor_info.rcMonitor.top,
                 monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                 monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
  }
  else
  {
    SetWindowLong(window, GWL_STYLE, _savedWindowInfo.style);
    SetWindowLong(window, GWL_EXSTYLE, _savedWindowInfo.ex_style);

    SetWindowPos(window, NULL,
                 _savedWindowInfo.rect.left,
                 _savedWindowInfo.rect.top,
                 _savedWindowInfo.rect.right - _savedWindowInfo.rect.left,
                 _savedWindowInfo.rect.bottom - _savedWindowInfo.rect.top,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

    if (_savedWindowInfo.maximized)
      ::SendMessage(window, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
  }
}

