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

#include "humblelogging/api.h"

HUMBLE_LOGGER(logger, "PluginManagement");

class PluginEntry
{
public:
  /*
  PluginEntry(Application* app)
    : creator(nullptr),
      onload(false)
  {
  }
  */
  PluginEntry(CefRefPtr<Application> app, PluginCreator* creator, bool onLoad)
    : _app(app),
      creator(creator),    
      onload(onLoad)
  {
  }

  ~PluginEntry()
  {    
  }

  std::shared_ptr<CordovaPlugin> createPlugin()
  {
    // if not already created then create
    if(!plugin.get() && creator)
    {
      // create
      plugin = creator->create();
      // and call initialize
      plugin->initialize(_app);
    }
    return plugin;
  }
  PluginCreator* creator;
  std::shared_ptr<CordovaPlugin> plugin;
  bool onload;

private:
  CefRefPtr<Application> _app;
};

PluginManager::PluginManager(CefRefPtr<Application> app)
  : _app(app),
    _firstRun(true)
{
}

PluginManager::~PluginManager()
{  
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
  PluginCreator* creator = PluginFactory::getPluginCreator(servicename);
  if(creator)
  {
    if(_pluginEntries.find(servicename) == _pluginEntries.end()) // not found
    {
      _pluginEntries[servicename] = std::make_shared<PluginEntry>(_app, creator, onload);
    }
    else
    {
      HL_ERROR(logger, "Plugin '" + servicename + "' already added");
    }
  }
  else
  {
    HL_ERROR(logger, "Plugin '" + servicename + "' not registered with classname '" + classname + "'");
  }
}

void PluginManager::clearPluginObjects()
{  
  for(PluginMap::iterator iter = _pluginEntries.begin(); iter != _pluginEntries.end(); ++iter)
  {
    if(iter->second->plugin.get())
      iter->second->plugin = nullptr;
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

std::shared_ptr<CordovaPlugin> PluginManager::getPlugin( const std::string& service )
{
  PluginMap::iterator iter = _pluginEntries.find(service);
  if(iter == _pluginEntries.end())
    return NULL;
  std::shared_ptr<CordovaPlugin> plugin = iter->second->plugin;
  if(!plugin.get()) // create it
  {
    plugin = iter->second->createPlugin();
  }
  return plugin;
}

void PluginManager::exec( const std::string& service, const std::string& action, const std::string& callbackId, const std::string& rawArgs )
{
  std::shared_ptr<CordovaPlugin> plugin = getPlugin(service);
  if(!plugin.get())
  {
    HL_ERROR(logger, "exec() call to unknown plugin: " + service);    
    _app->sendPluginResult(std::make_shared<PluginResult>(PluginResult::CLASS_NOT_FOUND_EXCEPTION, "exec() call to unknown plugin: " + service), callbackId);
    return;
  }

  
  CallbackContext cbCtx(callbackId, _app);
  Json::Value jsonArgs;
  Json::Reader reader;
  reader.parse(rawArgs, jsonArgs);
  // TODO: time measurement
  bool wasValidAction = plugin->execute(action, jsonArgs, cbCtx);
  // TODO: time measurement

  if(!wasValidAction){
    _app->sendPluginResult(std::make_shared<PluginResult>(PluginResult::INVALID_ACTION), callbackId);
  }
}
