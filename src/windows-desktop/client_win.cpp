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

#include "client_win.h"
#include "common/util.h"


#include <Winuser.h>

Client_Win::Client_Win()
  :INIT_LOGGER(Client_Win)
{

}

Client_Win::~Client_Win()
{

}

bool Client_Win::OnKeyEvent( CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event )
{
  // see chrome shortcuts: https://support.google.com/chrome/topic/25799?hl=en
  // see windows key codes: http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
  REQUIRE_UI_THREAD();
  LOG_TRACE(logger()) 
    << "OnKeyEvent, type=" << event.type
    << " modifiers=" << event.modifiers
    << " windows_key_code=" << event.windows_key_code
    << " unmodified_character=" << event.unmodified_character;

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

