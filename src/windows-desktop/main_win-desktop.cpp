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

#include "common/logging.h"

#include <Windows.h>
#include "include/cef_base.h"

#include "application_win.h"
#include "client_win.h"
#include "helper_win.h"

int CALLBACK WinMain(
  _In_  HINSTANCE hInstance,
  _In_  HINSTANCE hPrevInstance,
  _In_  LPSTR lpCmdLine,
  _In_  int nCmdShow
  )
{
  {
    std::shared_ptr<Helper::Paths> paths = std::make_shared<Helper::Paths_Win>();

    // first init the logging system
    init_logging();

    // passing command-line
    CefMainArgs main_args(hInstance);

    // create app
    CefRefPtr<CefApp> app = new Application_Win(paths);


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
  //#ifdef _DEBUG
    appsettings.single_process = true;
  //#endif

    // Initialize CEF in the main process.
    CefInitialize(main_args, appsettings, app);
  
    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
    CefRunMessageLoop();

  }
  // Shut down CEF.
  CefShutdown();
  return 0;
}

