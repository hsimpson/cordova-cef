#ifndef application_win_h__
#define application_win_h__

#include "../common/application.h"

class Application_Win : public Application 
{
public:
  Application_Win() {}
  virtual ~Application_Win() {}

  IMPLEMENT_REFCOUNTING(Application_Win);
};
#endif // application_win_h__
