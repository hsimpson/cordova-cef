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


#include "pluginregistry.h"

PluginRegistry::PluginClass2CreatorFunc PluginRegistry::_registeredPlugins;

bool PluginRegistry::registerPlugin(const std::string& name, PluginRegistry::plugin_creator_func create_func)
{
  if(_registeredPlugins.find(name) == _registeredPlugins.end())
  {
    _registeredPlugins[name] = create_func;
    return true;
  }
  return false;
}

PluginRegistry::plugin_creator_func PluginRegistry::getPluginCreateFunc(const std::string& name)
{
  PluginRegistry::PluginClass2CreatorFunc::iterator iter = _registeredPlugins.find(name);
  if(iter != _registeredPlugins.end())
    return iter->second;
  else
    return NULL;
}