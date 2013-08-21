#include "client_win.h"
#include "../common/util.h"

bool Client_Win::OnKeyEvent( CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event )
{
  // see chrome shortcuts: https://support.google.com/chrome/topic/25799?hl=en
  // see windows key codes: http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
  REQUIRE_UI_THREAD();

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
  else if(event.modifiers & EVENTFLAG_CONTROL_DOWN) // modifiers: CTRL
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
  else if(event.modifiers & EVENTFLAG_SHIFT_DOWN) // modifiers: CTRL
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
  else if( (event.modifiers & EVENTFLAG_SHIFT_DOWN) && (event.modifiers & EVENTFLAG_CONTROL_DOWN) ) // modifiers: SHIFT + CTRL
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
  

  return false;
}