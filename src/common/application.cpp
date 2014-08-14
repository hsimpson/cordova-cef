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
#include "client.h"

Application::Application(CefRefPtr<Client> client, std::shared_ptr<Helper::Paths> paths)
  : INIT_LOGGER(Application),
    _client(client),
    _paths(paths),    
    _jsMessageQueue(this),
    _mainWindow(NULL),
    _pluginManager(new PluginManager(this))
{
  // load and parse config and fill configured plugins
  boost::filesystem::path config_path = _paths->getApplicationDir();
  config_path /= "config.xml";
  _config = new Config(config_path, _pluginManager);
}

Application::~Application()
{

}


void Application::InitializeAppData()
{

//Creating the file systems to be used with the file plugins in AppData. 

	try {

		boost::filesystem::path persistent_root_path = _paths->getPersistentFsDir();
		boost::filesystem::path temporary_root_path = _paths->getTemporaryFsDir();
		boost::filesystem::path db_path = _paths->getDbDir();

		if (!boost::filesystem::exists(persistent_root_path))
			boost::filesystem::create_directories(persistent_root_path);

		if (!boost::filesystem::exists(temporary_root_path))
			boost::filesystem::create_directories(temporary_root_path);

		if (!boost::filesystem::exists(db_path))
			boost::filesystem::create_directories(db_path);

	}
	catch (const boost::filesystem::filesystem_error e) {
		LOG_TRACE(logger()) << "Failed to create AppData directories: " << e.code().message();
	}

}

void Application::MoveWebAppToTempFs() {
  boost::filesystem::path src_path = _paths->getApplicationDir();
  boost::filesystem::path dest_path(_paths->getTemporaryFsDir());

		//Copy the the contents of www

		src_path /= "www";

		dest_path /= "www";

		if (boost::filesystem::exists(dest_path))
			boost::filesystem::remove_all(dest_path);

		Helper::copyDir(src_path, dest_path);

}


void Application::OnContextInitialized()
{ 

//  InitializeAppData();

  
  std::string startup_document = _config->startDocument();

  // test if absolute url with http or https
  if(boost::starts_with(startup_document, "http://") || boost::starts_with(startup_document, "https://"))
  {
    _startupUrl = startup_document;
  }
  else {
    std::string www_dir;
	bool copyWebApp;
	if (_config->getBoolPreference("copyWebApp", copyWebApp)) {
		MoveWebAppToTempFs();
		www_dir = _paths->getTemporaryFsDir().generic_string();
	}
	else {
	    www_dir = _paths->getApplicationDir().generic_string();
	}
    www_dir += "/www";
    if(!boost::starts_with(www_dir, "/"))
      www_dir = "/" + www_dir;
    _startupUrl = "file://" + www_dir + "/" + startup_document; 
  }



  

  CefWindowInfo info;
  bool transparent = true;

  bool offscreenrendering = false;
  config()->getBoolPreference("OffScreenRendering", offscreenrendering);
  

  if(offscreenrendering) 
  {
    CefRefPtr<Client::RenderHandler> osr_window = createOSRWindow(_mainWindow, _client.get(), transparent);
    _client->setOSRHandler(osr_window);
    /* old
    info.SetTransparentPainting(transparent ? true : false);
    info.SetAsOffScreen(osr_window->handle());
    */
    info.SetAsWindowless(osr_window->handle(), transparent);
  }
  else
  {
    RECT rect;
    GetClientRect(_mainWindow, &rect);
    info.SetAsChild(_mainWindow, rect);
  }

  
  /*
  RECT r;
  r.left = 0; r.top = 0; r.right = 700; r.bottom = 500;
  info.SetAsChild(_mainWindow, r);
  */
  
  //TODO: move the settings into config.xml
  CefBrowserSettings browserSettings;
  //browserSettings.developer_tools = STATE_ENABLED;
  browserSettings.file_access_from_file_urls = STATE_ENABLED;
  browserSettings.universal_access_from_file_urls = STATE_ENABLED;
  browserSettings.web_security = STATE_DISABLED;

  //WORLDAPP modification: default browser background
  // HACK: we have to set black background to avoid white flashes.
  // 'aHRtbCxib2R5e2JhY2tncm91bmQtY29sb3I6d2hpdGU7fQ==' stands for 'html,body{background-color:white;}'.
  // Ym9keXtiYWNrZ3JvdW5kLWNvbG9yOndoaXRlO30= stands for body{background-color:white;}
  //LPCSTR strCss = "data:text/css;charset=utf-8;base64,Ym9keXtiYWNrZ3JvdW5kLWNvbG9yOndoaXRlO30=";
  //CefString(&browserSettings.user_style_sheet_location).FromASCII(strCss);

  // init plugin manager (also create the plugins with onload=true)
  _pluginManager->init();

  // Create the browser asynchronously and load the startup url
  LOG_DEBUG(logger()) << "create browser with startup url: '" << _startupUrl << "'";
  CefBrowserHost::CreateBrowser(info, _client.get(), _startupUrl, browserSettings, NULL);
}

void Application::OnContextCreated( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context )
{
  CefRefPtr<CefV8Value> global = context->GetGlobal();
  _exposedJSObject = CefV8Value::CreateObject(NULL);
  CefRefPtr<CefV8Value> exec = CefV8Value::CreateFunction("exec", this);
  _exposedJSObject->SetValue("exec", exec, V8_PROPERTY_ATTRIBUTE_READONLY);
  global->SetValue("_cordovaNative", _exposedJSObject, V8_PROPERTY_ATTRIBUTE_READONLY);
}

void Application::OnContextReleased( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context )
{
  
}

bool Application::Execute( const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception )
{
  if(name == "exec" && arguments.size() == 4)
  {
    std::string service = arguments[0]->GetStringValue().ToString();
    std::string action  = arguments[1]->GetStringValue().ToString();
    std::string callbackId = arguments[2]->GetStringValue().ToString();
    std::string rawArgs = arguments[3]->GetStringValue().ToString();
    _pluginManager->exec(service, action, callbackId, rawArgs);
    return true;
  }
  return false;
}

void Application::sendJavascript( const std::string& statement )
{
  _jsMessageQueue.addJavaScript(statement);
}

void Application::sendPluginResult( std::shared_ptr<const PluginResult> pluginResult, const std::string& callbackId )
{
  _jsMessageQueue.addPluginResult(pluginResult, callbackId);
}

void Application::runJavaScript( const std::string& js )
{
  _client->runJavaScript(js);
}

void Application::handlePause()
{
  runJavaScript("cordova.fireDocumentEvent('pause');");
}

void Application::handleResume()
{
  runJavaScript("cordova.fireDocumentEvent('resume');");
}

std::shared_ptr<Helper::Paths> Application::getPaths()
{
	return _paths;
}

CefWindowHandle Application::getHandle()
{
	return _mainWindow;
}