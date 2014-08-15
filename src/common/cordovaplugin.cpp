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

#include "application.h"
#include "cordovaplugin.h"
#include "json/json.h"

CordovaPlugin::CordovaPlugin()
{
}

CordovaPlugin::~CordovaPlugin()
{
}

void CordovaPlugin::initialize(CefRefPtr<Application> app)
{
  _app = app;
}

CefRefPtr<Application> CordovaPlugin::getApp() const
{
  return _app;
}

PluginCreator::PluginCreator(const std::string& pluginname)
{
  PluginFactory::registerit(pluginname, this);
}

void PluginFactory::registerit(const std::string& classname, PluginCreator* creator)
{
  PluginCreatorMapT::iterator i;
  i = get_table().find(classname);
  if(i == get_table().end()) // not found
    get_table()[classname] = creator;
}

std::shared_ptr<CordovaPlugin> PluginFactory::create(const std::string& classname)
{
  PluginCreatorMapT::iterator i;
  i = get_table().find(classname);

  if(i != get_table().end())
    return i->second->create();
  else
    return nullptr;
}

PluginFactory::PluginCreatorMapT& PluginFactory::get_table()
{
  static PluginCreatorMapT table;
  return table;
}

PluginCreator* PluginFactory::getPluginCreator( const std::string& pluginname )
{
  PluginCreatorMapT::iterator i;
  i = get_table().find(pluginname);

  if(i != get_table().end())
    return i->second;
  return nullptr;
}



