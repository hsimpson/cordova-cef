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

#import "include/cef_application_mac.h"
#include "common/logging.h"
#include "application_osx.h"
#include "helper_osx.h"

// Provide the CefAppProtocol implementation required by CEF.
@interface ClientApplication : NSApplication<CefAppProtocol> {
@private
  BOOL handlingSendEvent_;
}
@end

@implementation ClientApplication
- (BOOL)isHandlingSendEvent {
  return handlingSendEvent_;
}

- (void)setHandlingSendEvent:(BOOL)handlingSendEvent {
  handlingSendEvent_ = handlingSendEvent;
}

- (void)sendEvent:(NSEvent*)event {
  CefScopedSendingEvent sendingEventScoper;
  [super sendEvent:event];
}
@end


// Receives notifications from the application. Will delete itself when done.
@interface ClientAppDelegate : NSObject
- (void)createApp:(id)object;
@end

@implementation ClientAppDelegate

// Create the application on the UI thread.
- (void)createApp:(id)object
{
  [NSApplication sharedApplication];
  [NSBundle loadNibNamed:@"MainWindow" owner:NSApp];
  
  // Set the delegate for application events.
  [NSApp setDelegate:self];
}

@end

int main(int argc, char * argv[])
{
  
  std::shared_ptr<Helper::Paths> paths = std::make_shared<Helper::Paths_OSX>();
  
  // first init the logging system
  init_logging();
  
  // passing command-line
  CefMainArgs main_args(argc, argv);
  
  // create app
  CefRefPtr<CefApp> app = new Application_OSX(paths);
  
  // Execute the secondary process, if any.
  int exit_code = CefExecuteProcess(main_args, app.get());
  if (exit_code >= 0)
    return exit_code;
  
  @autoreleasepool
  {
    // Initialize the ClientApplication instance.
    [ClientApplication sharedApplication];
    
    // Populate this structure to customize CEF behavior.
    CefSettings appsettings;
    appsettings.remote_debugging_port = 9999;
    
//#ifdef _DEBUG
    // use single process for now.
    appsettings.single_process = true;
//#endif
    
    // Initialize CEF in the main process.
    CefInitialize(main_args, appsettings, app);
    
    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
    CefRunMessageLoop();
    
    //return NSApplicationMain(argc, argv);
  
    // Shut down CEF.
    CefShutdown();
  }
  return 0;
}
