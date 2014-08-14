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

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR    lpCmdLine,
                      int       nCmdShow) 
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  {
    std::shared_ptr<Helper::Paths> paths = std::make_shared<Helper::Paths_Win>();

    // first init the logging system
	// WorldAPP log file path
    init_logging(paths->getAppDataDir());


    void* sandbox_info = NULL;


    // passing command-line
    CefMainArgs main_args(hInstance);

    // create app
    CefRefPtr<Application> app = new Application_Win(paths);


    // Execute the sub-process logic, if any. This will either return immediately for the browser
    // process or block until the sub-process should exit.
    int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
    if (exit_code >= 0) {
      // The sub-process terminated, exit now.
      return exit_code;
    }

    //TODO: move the settings into config.xml
    // Populate this structure to customize CEF behavior.
    CefSettings appsettings;

	//Set the path where the cache should be stored to save cross-session data
//	appsettings.cache_path = *cefPath;
    cef_string_t* cef_str = cef_string_userfree_utf16_alloc();
    cef_string_utf8_to_utf16( paths->getCacheDir().generic_string().c_str(), paths->getCacheDir().generic_string().length(), cef_str );
	appsettings.cache_path = *cef_str;

	//WorldAPP put the cef log file into the standard directory
    cef_string_t* path_str = cef_string_userfree_utf16_alloc();
	boost::filesystem::path log_path = paths->getAppDataDir() / "cef.log";
	cef_string_utf8_to_utf16( log_path.generic_string().c_str(), log_path.generic_string().length(), path_str );
	appsettings.log_file = *path_str;
	

 //#ifdef _DEBUG
    // use single process for now.
    appsettings.single_process = true;
 //#endif

    bool offscreenrendering = false;
    app->config()->getBoolPreference("OffScreenRendering", offscreenrendering);
    appsettings.windowless_rendering_enabled = offscreenrendering;

    // Initialize CEF in the main process.
    CefInitialize(main_args, appsettings, app.get(), sandbox_info);
  
    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
    CefRunMessageLoop();

  }
  // Shut down CEF.
  CefShutdown();
  return 0;
}

