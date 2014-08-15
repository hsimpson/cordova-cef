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

#include "helper_win.h"
#include <Windows.h>
#include <Shlobj.h>

Helper::Paths_Win::Paths_Win()
{

}

Helper::Paths_Win::~Paths_Win()
{

}

Helper::Path Helper::Paths_Win::getExecutablePath() const
{
  wchar_t path[MAX_PATH];
  Helper::Path ret;
  if(GetModuleFileNameW( NULL, path, MAX_PATH ))
    ret = path;
  return ret;
}

Helper::Path Helper::Paths_Win::getHomeDir() const
{
  wchar_t path[MAX_PATH];
  Helper::Path ret;
  if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
    ret = path;
  return ret;
}

Helper::Path Helper::Paths_Win::getTempDir() const
{
  wchar_t path[MAX_PATH];
  Helper::Path ret;
  if(SUCCEEDED(GetTempPath(MAX_PATH,  path)))
    ret = path;
  return ret;
}
