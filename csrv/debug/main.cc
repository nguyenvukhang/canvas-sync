#include <csrv.h>
#include <iostream>

using namespace std;

int main()
{
  canvas::Server server(CanvasApi::get_token_from_env(),
                        "https://canvas.nus.edu.sg");
  server.run_debug();
  return 0;
}
