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

#define NATIVE_SEPERATOR '/'
#define WIN_SEPERATOR '\\'

  class Path
  {
  public:
    // constructs an empty path
    Path();

    // constructs a path from UTF-16 string
    Path(const std::wstring& path);

    // constructs a path from UTF-8 string
    Path(const std::string& path);

    // operators
    Path& operator=  (const std::wstring& other);
    Path& operator=  (const Path& other);
    Path& operator+= (const std::string& other);
    Path  operator+  (const std::string& other) const;


    // get the dir of this path, if it is a already a dir then it returns a copy of itself
    Path dir() const;
    // returns true if this path is a dir
    bool isDir() const;
    // returns true if this Path is a file
    bool isFile() const;
    // returns the complete file path
    std::string filePath() const;
    // get the parent
    Path parentPath() const;
    // get the file name of the file with extension
    std::string fileName() const;    
    // get the base name of the file without extension
    std::string baseName() const;

  private:

    // set the path to slash separators
    void setToSlashSeparators();

    // stores the complete path as UTF-8
    std::string _path;
  };


  class StringUtils
  {
  public:
    static bool iequals(const std::string& str1, const std::string& str2);
    static bool starts_with(const std::string& str1, const std::string& str2);
  };

  class PathManager
  {
  public:
    PathManager();
    virtual ~PathManager();
    Helper::Path getApplicationDir() const;
    Helper::Path getAppDataDir() const;

  protected:
    virtual Helper::Path getExecutablePath() const = 0;
    virtual Helper::Path getHomeDir() const = 0;
    virtual Helper::Path getTempDir() const = 0;

  private:

  };
} // namespace Helper
#endif // helper_h__