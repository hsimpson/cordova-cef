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

#include "pluginmanager.h"
#include "pluginresult.h"
#include "application.h"
#include "json/json.h"

class PluginEntry
{
public:
  PluginEntry()
    : creator_func(NULL),
    plugin(NULL),
    onload(false)
  {
  }
  PluginEntry(PluginRegistry::plugin_creator_func creatorFunc, bool onLoad)
    : creator_func(creatorFunc),
    plugin(NULL),
    onload(onLoad)
  {
  }

  ~PluginEntry()
  {
    if(plugin)
    {
      delete plugin;
      plugin = NULL;
    }
  }

  CordovaPlugin* createPlugin()
  {
    // if not already created then create
    if(!plugin && creator_func)
    {
      // create
      plugin = creator_func();
      // and call initialize
      plugin->initialize();
    }
    return plugin;
  }

  PluginRegistry::plugin_creator_func creator_func;
  CordovaPlugin* plugin;
  bool onload;
};

PluginManager::PluginManager(Application* app)
  : INIT_LOGGER(PluginManager),
    _app(app),
    _firstRun(true)
{
}

PluginManager::~PluginManager()
{
  for(PluginMap::iterator iter = _pluginEntries.begin(); iter != _pluginEntries.end(); ++iter)
  {
    delete iter->second;
  }
}

void PluginManager::init()
{
  if(_firstRun)
  {
    _firstRun = false;
  }
  else
  {
    clearPluginObjects();
  }
  
  // Start up all plugins that have onload specified
  startupPlugins();
}

void PluginManager::addPlugin(const std::string& servicename, const std::string& classname, bool onload)
{
  PluginRegistry::plugin_creator_func creatorFunc = PluginRegistry::getPluginCreateFunc(classname);
  if(creatorFunc)
  {
    if(_pluginEntries.find(servicename) == _pluginEntries.end()) // not found
      _pluginEntries[servicename] = new PluginEntry(creatorFunc, onload);
    else
      BOOST_LOG_SEV(logger(), error) << "Plugin '" << servicename << "' already added";
  }
  else
  {
    BOOST_LOG_SEV(logger(), error) << "Plugin '" << servicename << "' not registered with classname '" << classname << "'";
  }
}

void PluginManager::clearPluginObjects()
{  
  for(PluginMap::iterator iter = _pluginEntries.begin(); iter != _pluginEntries.end(); ++iter)
  {
    if(iter->second->plugin)
      delete iter->second->plugin;
  }
}

void PluginManager::startupPlugins()
{
  for(PluginMap::iterator iter = _pluginEntries.begin(); iter != _pluginEntries.end(); ++iter)
  {
    if(iter->second->onload)
     iter->second->createPlugin();
  }
}

CordovaPlugin* PluginManager::getPlugin( const std::string service )
{
  PluginMap::iterator iter = _pluginEntries.find(service);
  if(iter == _pluginEntries.end())
    return NULL;
  CordovaPlugin* plugin = iter->second->plugin;
  if(plugin == NULL) // create it
  {
    plugin = iter->second->createPlugin();
  }
  return plugin;
}

void PluginManager::exec( const std::string& service, const std::string& action, const std::string& callbackId, const std::string& rawArgs )
{
  CordovaPlugin* plugin = getPlugin(service);
  if(plugin == NULL)
  {
    BOOST_LOG_SEV(logger(), error) << "exec() call to unknown plugin: " << service;
    PluginResult cr(PluginResult::CLASS_NOT_FOUND_EXCEPTION);
    _app->sendPluginResult(cr, callbackId);
    return;
  }

  
  CallbackContext cbCtx(callbackId, _app);
  Json::Value jsonArgs;
  Json::Reader reader;
  reader.parse(rawArgs, jsonArgs);
  // ToDo: time measurement
  bool wasValidAction = plugin->execute(action, jsonArgs, cbCtx);
  // ToDo: time measurement

  if(!wasValidAction){
    PluginResult cr(PluginResult::INVALID_ACTION);
    _app->sendPluginResult(cr, callbackId);
  }
}
