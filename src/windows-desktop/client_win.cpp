#include "client_win.h"
#include "../common/util.h"

bool Client_Win::OnKeyEvent( CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event )
{
  REQUIRE_UI_THREAD();

  // SHIFT + CTRL
  if( (event.modifiers & EVENTFLAG_SHIFT_DOWN) && (event.modifiers & EVENTFLAG_CONTROL_DOWN) )
  {
    switch (event.windows_key_code)
    {
      case 0x49:
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