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

#include "application.h"



Application::Application(CefRefPtr<CefClient> client)
  : _client(client)
{
}

Application::~Application()
{

}

void Application::OnContextInitialized()
{
  CefWindowInfo info;
  info.SetAsPopup(NULL, L"Cordova CEF");

  CefBrowserSettings browserSettings;
  browserSettings.file_access_from_file_urls = STATE_ENABLED;
  browserSettings.universal_access_from_file_urls = STATE_ENABLED;
  browserSettings.web_security = STATE_DISABLED;

  // Create the browser asynchronously. Initially loads the Google URL.
  //CefBrowserHost::CreateBrowser(info, _client, "http://www.google.de", browserSettings);
  CefBrowserHost::CreateBrowser(info, _client, _startupUrl, browserSettings);
}
