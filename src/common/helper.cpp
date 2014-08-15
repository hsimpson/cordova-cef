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

#include "helper.h"
#include <sys/stat.h>
#include <algorithm>

namespace Helper {
  
Path::Path()
{

}

Path::Path(const std::wstring& path)
  :_path(wideToUtf8(path))
{
  setToSlashSeparators();
}
Path::Path(const std::string& path)
  :_path(path)
{
  setToSlashSeparators();
}

Path& Path::operator=( const std::wstring& other )
{
  _path = wideToUtf8(other);
  setToSlashSeparators();
  return *this;
}

Path& Path::operator= (const Path& other)
{
  _path = other._path;
  return *this;
}

Path& Path::operator+=( const std::string& other )
{
  _path += NATIVE_SEPERATOR + other;
  setToSlashSeparators();
  return *this;
}

Path Path::operator+( const std::string& other ) const
{
  Path p(_path + NATIVE_SEPERATOR + other);
  p.setToSlashSeparators();
  return p;
}

Path Path::dir() const
{
  Path p;
  if(isDir()) 
  {
    p = *this;
  } 
  else 
  {
    std::string::size_type n = _path.rfind(NATIVE_SEPERATOR);
    if(n != std::string::npos)
    {
      p = Path(_path.substr(0, n));
    }
  }
  return p;
}

bool Path::isDir() const
{
  struct stat s;
  stat(_path.c_str(), &s);
  if(s.st_mode & S_IFDIR)
    return true;
  
  return false;
}

bool Path::isFile() const
{
  struct stat s;
  stat(_path.c_str(), &s);
  if(s.st_mode & S_IFREG )
    return true;

  return false;
}

std::string Path::filePath() const
{
  return _path;
}

void Path::setToSlashSeparators()
{
  std::replace(_path.begin(), _path.end(), WIN_SEPERATOR, NATIVE_SEPERATOR);
}

Path Path::parentPath() const
{
  Path p;
  std::string::size_type n = _path.rfind(NATIVE_SEPERATOR);
  if(n != std::string::npos)
  {
    p = Path(_path.substr(0, n));
  }
  return p;
}

std::string Path::fileName() const
{
  std::string ret;
  if(isFile())
  {
    std::string::size_type n = _path.rfind(NATIVE_SEPERATOR);
    if(n != std::string::npos)
    {
      ret = _path.substr(n+1);
    }
  }
  return ret;
}

std::string Path::baseName() const
{
  std::string ret = fileName();
  if(!ret.empty())
  {
    std::string::size_type n = ret.find('.');
    if(n != std::string::npos)
    {
      ret = ret.substr(0, n);
    }
  }
  return ret;
}




PathManager::PathManager()
{

}

PathManager::~PathManager()
{

}

Path PathManager::getApplicationDir() const
{
  return getExecutablePath().parentPath();
}

Path PathManager::getAppDataDir() const
{
  Path ret = getHomeDir();
  ret += getExecutablePath().baseName();
  return ret;
}

bool StringUtils::iequals( const std::string& str1, const std::string& str2 )
{
  if(str1.size() != str2.size())
    return false;

  return (stricmp(str1.c_str(), str2.c_str()) == 0);
}

bool StringUtils::starts_with( const std::string& str1, const std::string& str2 )
{
  std::string::size_type n = str1.find(str2);
  return (n == 0);
}

} // namespace Helper