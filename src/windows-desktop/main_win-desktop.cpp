#include <Windows.h>
#include "include/cef_base.h"

#include "application_win.h"
#include "client_win.h"

int CALLBACK WinMain(
  _In_  HINSTANCE hInstance,
  _In_  HINSTANCE hPrevInstance,
  _In_  LPSTR lpCmdLine,
  _In_  int nCmdShow
  )
{
  // passing command-line
  CefMainArgs main_args(hInstance);

  // create app
  CefRefPtr<CefApp> app = new Application_Win;


  // Execute the sub-process logic, if any. This will either return immediately for the browser
  // process or block until the sub-process should exit.
  int exit_code = CefExecuteProcess(main_args, app);
  if (exit_code >= 0) {
    // The sub-process terminated, exit now.
    return exit_code;
  }

  // Populate this structure to customize CEF behavior.
  CefSettings appsettings;
  appsettings.remote_debugging_port = 9999;

  // Initialize CEF in the main process.
  CefInitialize(main_args, appsettings, app);

  CefWindowInfo info;
  info.SetAsPopup(NULL, L"Cordova-windows-desktop");

  CefBrowserSettings browsersettings;

  // CefClient implementation.
  CefRefPtr<CefClient> client = new Client_Win;

  // Create the browser asynchronously. Initially loads the Google URL.
  CefBrowserHost::CreateBrowser(info, client, "http://www.google.de", browsersettings);

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}

