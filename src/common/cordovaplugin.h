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
#include "pluginregistry.h"

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

protected: 
  // protected constructor and assignment
  CordovaPlugin();
  CordovaPlugin(const CordovaPlugin& other) {}
  CordovaPlugin& operator=(const CordovaPlugin& other) {}
  
};

#define DECLARE_PLUGIN(clazzname)\
  private:\
    clazzname();\
    clazzname(const clazzname& other);\
    clazzname& operator=(const clazzname& other);\
  public:\
    static CordovaPlugin* createPlugin()\
    {\
      return new clazzname();\
    }\

#define REGISTER_PLUGIN(clazzname)\
  PluginRegistry::plugin_creator_func creator = &##clazzname::createPlugin;\
  bool ret = PluginRegistry::registerPlugin(#clazzname, creator);\


#endif // cordovaplugin_h__