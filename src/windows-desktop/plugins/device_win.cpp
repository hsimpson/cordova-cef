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

#include "device_win.h"
#include "common/helper.h"
#include <Windows.h>
#include <sstream>

REGISTER_PLUGIN(Device_Win, Device);

Device_Win::~Device_Win()
{

}

std::string Device_Win::getVersion()
{
  OSVERSIONINFOW osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOW));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
  std::string ret = "";
  if(GetVersionExW(&osvi))
  {
    std::stringstream ss;
    ss << osvi.dwMajorVersion << '.' << osvi.dwMinorVersion << '.' << osvi.dwBuildNumber;
    ret = ss.str();
  }
  return ret;
}

std::string Device_Win::getPlatform()
{
  return "CEF-win-desktop";
}

std::string Device_Win::getModel()
{
  std::string ret = "";
  DWORD length = MAX_COMPUTERNAME_LENGTH+1;
  wchar_t* buffer = new wchar_t[length];
  if(GetComputerNameW(buffer, &length))
  {
    ret = Helper::wideToUtf8(buffer);   
  }
  delete[] buffer;
  return ret;
}

std::string Device_Win::getUuid()
{
  GUID guid;
  CoCreateGuid(&guid);
  BYTE* str;
  UuidToStringA((UUID*)&guid, &str);

  std::string unique((char*)str);
  RpcStringFreeA(&str);

  return unique;
}
