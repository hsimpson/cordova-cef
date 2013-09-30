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

#ifndef cordovaplugin_h__
#define cordovaplugin_h__


#include "callbackcontext.h"
#include <memory>

namespace Json
{
  class Value;
}

class CordovaPlugin
{
public:
  virtual ~CordovaPlugin();

  virtual bool execute(const std::string& action, const Json::Value& args, CallbackContext& callbackContext) = 0;
  virtual void initialize();  
};

class PluginCreator : public std::enable_shared_from_this<PluginCreator>
{
public:
  PluginCreator(const std::string& pluginname);
  virtual ~PluginCreator() {};

  virtual std::shared_ptr<CordovaPlugin> create() = 0;
};

class PluginFactory
{
public:
  static PluginCreator* getPluginCreator(const std::string& pluginname);
  static std::shared_ptr<CordovaPlugin> create(const std::string& pluginname);
  static void registerit(const std::string& pluginname, PluginCreator* creator);
  typedef std::map<std::string, PluginCreator*> PluginCreatorMapT;
private:
  
  static PluginCreatorMapT& get_table();
};

template <class T>
class PluginCreatorImpl : public PluginCreator
{
public:
  PluginCreatorImpl<T>(const std::string& pluginname) : PluginCreator(pluginname) {}
  virtual ~PluginCreatorImpl<T>() {}

  virtual std::shared_ptr<CordovaPlugin> create() { return std::make_shared<T>(); }
};



#define DECLARE_PLUGIN(classname)\
  private:\
    static const PluginCreatorImpl<classname> plugincreator;

#define REGISTER_PLUGIN(classname, pluginname)\
  const PluginCreatorImpl<classname> classname::plugincreator(#pluginname);




#endif // cordovaplugin_h__