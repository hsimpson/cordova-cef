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

#include "nativetojsmessagequeue.h"
#include "application.h"
#include "humblelogging/api.h"
#include <sstream>

// Set this to true to force plugin results to be encoding as
// JS instead of the custom format (useful for benchmarking).
#define FORCE_ENCODE_USING_EVAL false

// This must match the default value in exec.js
#define DEFAULT_BRIDGE_MODE 1

// Arbitrarily chosen upper limit for how much data to send to JS in one shot.
// This currently only chops up on message boundaries. It may be useful
// to allow it to break up messages.
#define MAX_PAYLOAD_SIZE 50 * 1024 * 10240

HUMBLE_LOGGER(logger, "NativeToJsMessageQueue");

NativeToJsMessageQueue::NativeToJsMessageQueue(CefRefPtr<Application> app)
  : _app(app),
    _paused(false),
    _activeListenerIndex(0)
{
  std::shared_ptr<BridgeMode> nullmode;
  _registeredListeners.push_back(nullmode);
  _registeredListeners.push_back(std::make_shared<NativeToJsMessageQueue::CefCallBridgeMode>(*this));
  reset();
}

NativeToJsMessageQueue::~NativeToJsMessageQueue()
{
}

void NativeToJsMessageQueue::addJavaScript( const std::string& statement )
{
  enqueueMessage(std::make_shared<JsMessage>(statement));
}

void NativeToJsMessageQueue::addPluginResult( std::shared_ptr<const PluginResult> pluginResult, const std::string& callbackId )
{
  if(callbackId.empty())
  {
    HL_ERROR(logger, "Got plugin result with no callbackId");
    return;
  }
  bool noResult = (pluginResult->getStatus() == PluginResult::NO_RESULT);
  bool keepCallback = pluginResult->getKeepCallback();
  if(noResult && keepCallback)
    return;
  std::shared_ptr<JsMessage> message = std::make_shared<JsMessage>(pluginResult, callbackId);
  if(FORCE_ENCODE_USING_EVAL)
  {
    std::stringstream ss;
    message->encodeAsJsMessage(ss);
    message = std::make_shared<JsMessage>(ss.str());
  }
  enqueueMessage(message);
}

void NativeToJsMessageQueue::enqueueMessage( std::shared_ptr<JsMessage> message )
{
  {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    _queue.push_back(message);
    std::shared_ptr<BridgeMode> activeListener = _registeredListeners[_activeListenerIndex];
    if(!_paused && activeListener.get() != nullptr)
      activeListener->onNativeToJsMessageAvailable();
  }
}

void NativeToJsMessageQueue::setBridgeMode( size_t value )
{
  if (value >= _registeredListeners.size()) {
    std::stringstream ss;
    ss << "Invalid NativeToJsBridgeMode: " << value;
    HL_ERROR(logger, ss.str());
  } 
  else 
  {
    if(value != _activeListenerIndex)
    {
      std::stringstream ss;
      ss << "Set native->JS mode to: " << value;
      HL_DEBUG(logger, ss.str());
      _activeListenerIndex = value;
      std::shared_ptr<BridgeMode> activeListener = _registeredListeners[value];
      if(!_paused && !_queue.empty() && activeListener.get() != nullptr)
        activeListener->onNativeToJsMessageAvailable();
    }
  }
}

void NativeToJsMessageQueue::reset()
{
  {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    setBridgeMode(DEFAULT_BRIDGE_MODE);
  }
}

void NativeToJsMessageQueue::setPaused( bool value )
{
  if (_paused && value) {
    // This should never happen. If a use-case for it comes up, we should
    // change pause to be a counter.
    HL_ERROR(logger, "nested call to setPaused detected.");
    return;
  }
  _paused = value;
  if (!value) 
  {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    std::shared_ptr<BridgeMode> activeListener = _registeredListeners[value];
    if(!_paused && !_queue.empty() && activeListener.get() != nullptr)
      activeListener->onNativeToJsMessageAvailable();
  }   
}

std::string NativeToJsMessageQueue::popAndEncodeAsJs()
{
  {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    if(_queue.empty())
      return "";

    int totalPayloadLen = 0;
    int numMessagesToSend = 0;

    std::deque<std::shared_ptr<JsMessage> >::const_iterator iter = _queue.begin();
    for(; iter != _queue.end(); ++iter)
    {
      int messageSize = (*iter)->calculateEncodedLength() + 50; // overestimate.
      if (numMessagesToSend > 0 && totalPayloadLen + messageSize > MAX_PAYLOAD_SIZE && MAX_PAYLOAD_SIZE > 0) {
        break;
      }
      totalPayloadLen += messageSize;
      numMessagesToSend += 1;
    }

    bool willSendAllMessages = numMessagesToSend == _queue.size();
    std::stringstream ret;

    for(int i=0; i<numMessagesToSend; ++i)
    {
      std::shared_ptr<JsMessage> message = _queue.front();
      _queue.pop_front();
      if (willSendAllMessages && (i + 1 == numMessagesToSend)) {
        message->encodeAsJsMessage(ret);
      } else {
        ret << "try{";
        message->encodeAsJsMessage(ret);
        ret << "}finally{";
      }
    }

    if (!willSendAllMessages) 
    {
      ret << "window.setTimeout(function(){cordova.require('cordova/plugin/cef/polling').pollOnce();},0);";
    }
    for (int i = willSendAllMessages ? 1 : 0; i < numMessagesToSend; ++i) 
    {
      ret << '}';
    }

    return ret.str();
  }
}



void NativeToJsMessageQueue::CefCallBridgeMode::onNativeToJsMessageAvailable()
{
  std::string js = _jsMessageQueue.popAndEncodeAsJs();
  _jsMessageQueue._app->runJavaScript(js);
}
