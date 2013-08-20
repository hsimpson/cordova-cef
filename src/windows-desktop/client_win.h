#ifndef client_win_h__
#define client_win_h__

#include "../common/client.h"

class Client_Win : public Client
{
public:
  Client_Win() {}
  virtual ~Client_Win() {}

  IMPLEMENT_REFCOUNTING(Client_Win);
};

#endif // client_win_h__
