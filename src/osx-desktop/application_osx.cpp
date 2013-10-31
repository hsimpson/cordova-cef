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

#include "application_osx.h"
#include "client_osx.h"


Application_OSX::Application_OSX(std::shared_ptr<Helper::Paths> paths)
  : INIT_LOGGER(Application_Win),
    Application(new Client_OSX, paths)
{
}

Application_OSX::~Application_OSX()
{
  
}

CefRefPtr<Client::RenderHandler> Application_OSX::createOSRWindow(CefWindowHandle parent, OSRBrowserProvider* browser_provider, bool transparent)
{
  return NULL;
}