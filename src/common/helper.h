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

  inline wchar_t* utf8ToWchar(const std::string& str)
  {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	std::wstring wstr = conv.from_bytes(str.data());
	const wchar_t* con = wstr.c_str();
	wchar_t* res = const_cast< wchar_t* >( con );
	return res;
  }

  inline bool copyDir(
    boost::filesystem::path const & source,
    boost::filesystem::path const & destination
)
{
    namespace fs = boost::filesystem;
    try
    {
        // Check whether the function call is valid
        if(
            !fs::exists(source) ||
            !fs::is_directory(source)
        )
        {
            std::cerr << "Source directory " << source.string()
                << " does not exist or is not a directory." << '\n'
            ;
            return false;
        }
        if(fs::exists(destination))
        {
            std::cerr << "Destination directory " << destination.string()
                << " already exists." << '\n'
            ;
            return false;
        }
        // Create the destination directory
        if(!fs::create_directory(destination))
        {
            std::cerr << "Unable to create destination directory"
                << destination.string() << '\n'
            ;
            return false;
        }
    }
    catch(fs::filesystem_error const & e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    // Iterate through the source directory
    for(
        fs::directory_iterator file(source);
        file != fs::directory_iterator(); ++file
    )
    {
        try
        {
            fs::path current(file->path());
            if(fs::is_directory(current))
            {
                // Found directory: Recursion
                if(
                    !copyDir(
                        current,
                        destination / current.filename()
                    )
                )
                {
                    return false;
                }
            }
            else
            {
                // Found file: Copy
                fs::copy_file(
                    current,
                    destination / current.filename()
                );
            }
        }
        catch(fs::filesystem_error const & e)
        {
            std:: cerr << e.what() << '\n';
        }
    }
    return true;
}

  class Paths
  {
  public:
    Paths();
    virtual ~Paths();
    boost::filesystem::path getApplicationDir() const;
    boost::filesystem::path getAppDataDir() const;
	boost::filesystem::path getPersistentFsDir() const;
	boost::filesystem::path getTemporaryFsDir() const;
	boost::filesystem::path getDbDir() const;
	boost::filesystem::path getCacheDir() const;



  protected:
    virtual boost::filesystem::path getExecutablePath() const = 0;
    virtual boost::filesystem::path getHomeDir() const = 0;

  private:
    mutable boost::filesystem::path _executablePath;
    mutable boost::filesystem::path _homedir;

  };
}
#endif // helper_h__