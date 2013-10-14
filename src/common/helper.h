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

#ifndef helper_h__
#define helper_h__

#include <string>
#include <locale>
#include <codecvt>
#include <boost/filesystem.hpp>

namespace Helper
{
  inline std::string wideToUtf8(const std::wstring& wstr)
  {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
  }

  inline std::wstring utf8ToWide(const std::string& str)
  {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(str.data());
  }

  class Paths
  {
  public:
    Paths();
    virtual ~Paths();
    boost::filesystem::path getApplicationDir() const;
    boost::filesystem::path getAppDataDir() const;

  protected:
    virtual boost::filesystem::path getExecutablePath() const = 0;
    virtual boost::filesystem::path getHomeDir() const = 0;

  private:
    mutable boost::filesystem::path _executablePath;
    mutable boost::filesystem::path _homedir;

  };
}
#endif // helper_h__