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

#ifndef nativetojsmessagequeue_h__
#define nativetojsmessagequeue_h__

#include "jsmessage.h"
#include "logging.h"
#include <boost/thread.hpp>
#include <deque>

class Application;

class NativeToJsMessageQueue
{
public:
  NativeToJsMessageQueue(Application* app);
  virtual ~NativeToJsMessageQueue();
  
  void addJavaScript(const std::string& statement);
  void addPluginResult(std::shared_ptr<const PluginResult> pluginResult, const std::string& callbackId);
  void enqueueMessage(std::shared_ptr<JsMessage> message);
  void setPaused(bool value);
  bool getPaused() const {return _paused;}

private:
  class BridgeMode 
  {
  public:
    BridgeMode(NativeToJsMessageQueue& jsMessageQueue) : _jsMessageQueue(jsMessageQueue) {}
    virtual void onNativeToJsMessageAvailable() = 0;
    virtual void notifyOfFlush(bool fromOnlineEvent) {}
  protected:
    NativeToJsMessageQueue& _jsMessageQueue;
  };

  class CefCallBridgeMode : public BridgeMode
  {
  public:
    CefCallBridgeMode(NativeToJsMessageQueue& jsMessageQueue) : BridgeMode(jsMessageQueue) {}
    virtual void onNativeToJsMessageAvailable();
  };

  void setBridgeMode(size_t value);
  void reset();
  std::string popAndEncodeAsJs();

  std::deque<std::shared_ptr<JsMessage> > _queue;
  boost::recursive_mutex _mutex;
  bool _paused;
  std::vector<std::shared_ptr<BridgeMode> > _registeredListeners;
  size_t _activeListenerIndex;
  Application* _app;

  DECLARE_LOGGER(NativeToJsMessageQueue);
};

#endif // nativetojsmessagequeue_h__
