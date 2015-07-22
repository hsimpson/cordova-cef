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


#include <Windows.h>
#include <iomanip>
#include "include/cef_base.h"
#include "include/cef_sandbox_win.h"
#include "helper_win.h"
#include "application_win.h"
#include "humblelogging/api.h"

#ifdef _DEBUG
#define DEFAULT_LOG_LEVEL humble::logging::LogLevel::All
#else
#define DEFAULT_LOG_LEVEL humble::logging::LogLevel::Error
#endif

// Set to 0 to disable sandbox support.
#define CEF_ENABLE_SANDBOX 0

#if CEF_ENABLE_SANDBOX
// The cef_sandbox.lib static library is currently built with VS2010. It may not
// link successfully with other VS versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif

HUMBLE_LOGGER(logger, "WinMain");

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR    lpCmdLine,
                      int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
  // DebugBreak();
#endif // _DEBUG


  // create the path manager
  std::shared_ptr<Helper::PathManager> pathManager =  std::make_shared<Helper::Paths_Win>();

  // init logging
  // init logging
  humble::logging::Factory& fac = humble::logging::Factory::getInstance();

  // Change the default LogLevel, which every NEW Logger will have.
  fac.setConfiguration(new humble::logging::SimpleConfiguration(DEFAULT_LOG_LEVEL));
  // Change the default Formatter (optional).
  fac.setDefaultFormatter(new humble::logging::PatternFormatter("[%date] [pid=%pid] [tid=%tid] [%lls] [line=%line] [file=%filename] %m\n"));
  // Add Appender which logs to file (rolling).
  Helper::Path logfile = pathManager->getAppDataDir() + "logging.log";
  humble::logging::RollingFileAppender* rfileAppender = new humble::logging::RollingFileAppender(logfile.filePath(), true, 5, 1024LL * 1024LL);
  fac.registerAppender(rfileAppender);

  std::stringstream ss;
  ss << "hPrvInstance=" << "0x" << std::hex << std::setfill('0') << std::setw(8) << hPrevInstance
     << ",hInstance="   << "0x" << std::hex << std::setfill('0') << std::setw(8) << hInstance;
  HL_DEBUG(logger, ss.str());

  void* sandbox_info = NULL;

#if CEF_ENABLE_SANDBOX
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  // passing command-line
  CefMainArgs main_args(hInstance);

  // create app
  CefRefPtr<Application_Win> app = new Application_Win(hInstance, pathManager);


  // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
  // that share the same executable. This function checks the command-line and,
  // if this is a sub-process, executes the appropriate logic.
  int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
  if (exit_code >= 0)
  {
    // The sub-process has completed so return here.
    return exit_code;
  }

  //TODO: move the settings into config.xml
  // Populate this structure to customize CEF behavior.
  // TODO: move to config.xml
  CefSettings appsettings;

#if !CEF_ENABLE_SANDBOX
  appsettings.no_sandbox = true;
#endif

  appsettings.remote_debugging_port = 9999;
  // TODO: get locale from system language
  CefString(&appsettings.locale) = "de_DE";
  CefString(&appsettings.cache_path) = app->pathManager()->getAppDataDir().filePath();

#ifdef _DEBUG
  // use single process for now.
  //appsettings.single_process = true;
#endif
  //appsettings.windowless_rendering_enabled = true;

  //bool offscreenrendering = false;

  //Application* myApp = static_cast<Application*>(app.get());
  //app->config()->getBoolPreference("OffScreenRendering", offscreenrendering);
  //appsettings.windowless_rendering_enabled = offscreenrendering;

  // Initialize CEF in the main process.
  CefInitialize(main_args, appsettings, app.get(), sandbox_info);

  // create the main window
  app->createMainWindow();

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();
  return 0;
}
