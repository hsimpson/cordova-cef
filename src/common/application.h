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

#ifndef application_h__
#define application_h__

#include "include/cef_app.h"
#include "include/cef_client.h"
#include "pugixml.hpp"

class Application : public CefApp,
                    public CefBrowserProcessHandler
{
public:
  Application(CefRefPtr<CefClient> client);
  virtual ~Application();

  // CefApp method(s)
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }

  //CefBrowserProcessHandler method(s)
  virtual void OnContextInitialized() OVERRIDE;

protected:

  virtual std::string getAppDirectory() = 0;

  CefRefPtr<CefClient> _client;

  std::string _applicationDir;
  std::string _startupUrl;
  pugi::xml_document _configXML;

  IMPLEMENT_REFCOUNTING(Application);
};
#endif // application_h__
