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

#ifndef pluginregistry_h__
#define pluginregistry_h__

#include <string>
#include <map>

class CordovaPlugin;

class PluginRegistry
{
public:
  typedef CordovaPlugin* (*plugin_creator_func)();
  typedef std::map<std::string, plugin_creator_func> PluginClass2CreatorFunc;
  static bool registerPlugin(const std::string& name, PluginRegistry::plugin_creator_func create_func);
  static PluginRegistry::plugin_creator_func getPluginCreateFunc(const std::string& name);

private:
  static PluginClass2CreatorFunc _registeredPlugins;
};

#endif // pluginregistry_h__