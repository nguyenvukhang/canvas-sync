#include <csrv.h>
#include <iostream>

using namespace std;

int main()
{
  canvas::Server server(CanvasApi::get_token_from_env(),
                        "https://canvas.nus.edu.sg");
  server.load_tree();
  cout << server.dump_tree() << endl;
  return 0;
}
