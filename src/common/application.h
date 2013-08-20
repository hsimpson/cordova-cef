#ifndef application_h__
#define application_h__

#include "include/cef_app.h"

class Application : public CefApp
{
public:
  Application() {}
  virtual ~Application() {}

  IMPLEMENT_REFCOUNTING(Application);
};
#endif // application_h__
