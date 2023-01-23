#include "csrv.h"

// for debugging purposes, contained within this library only.
int main()
{
  canvas::Server s(CanvasApi::get_token_from_env(),
                   "https://canvas.nus.edu.sg");
  string fresh = CanvasApi::get_token_from_env();
  s = s.with_token(&fresh);
  s.load_tree();
  cout << s.dump_tree() << endl;
  return 0;
}
