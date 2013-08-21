#ifndef client_h__
#define client_h__

#include "include/cef_client.h"
#include <list>
#include <set>

class Client : public CefClient,
                      CefLifeSpanHandler,
                      CefKeyboardHandler
{
public:
  Client();
  virtual ~Client();

  // CefClient method(s)
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {return this;}
  virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE { return this; }

  // CefLifeSpanHandler method(s)
  virtual bool OnBeforePopup( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access ) OVERRIDE;
  virtual void OnAfterCreated( CefRefPtr<CefBrowser> browser ) OVERRIDE;
  virtual bool DoClose( CefRefPtr<CefBrowser> browser ) OVERRIDE;
  virtual void OnBeforeClose( CefRefPtr<CefBrowser> browser ) OVERRIDE;

protected:

  virtual void showDevTools(CefRefPtr<CefBrowser> browser);

  // Number of currently existing browser windows. The application will exit
  // when the number of windows reaches 0.
  static int _browserCount;

  // The child browser window
  CefRefPtr<CefBrowser> _browser;

  // The child browser id
  int _browserId;

  // List of open DevTools URLs if not using an external browser window.
  std::set<std::string> _openDevToolsURLs;

  // List of any popup browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
  BrowserList _popupBrowsers;

  bool _bIsClosing;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(Client);
  // Include the default locking implementation.
  IMPLEMENT_LOCKING(Client);

};

#endif // client_h__