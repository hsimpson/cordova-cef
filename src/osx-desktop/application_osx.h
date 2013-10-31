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

#ifndef osx_desktop_application_osx_h
#define osx_desktop_application_osx_h

#include "common/application.h"
#include "common/logging.h"

class Application_OSX : public Application
{
public:
  Application_OSX(std::shared_ptr<Helper::Paths> paths);
  virtual ~Application_OSX();
  
  IMPLEMENT_REFCOUNTING(Application_OSX);
  
  DECLARE_LOGGER(Application_OSX);
  
  virtual CefRefPtr<Client::RenderHandler> createOSRWindow(CefWindowHandle parent, OSRBrowserProvider* browser_provider, bool transparent);
};


#endif
