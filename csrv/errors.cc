#include "errors.h"

void panic(const char *err_msg) {
  std::cerr << "[error] " << err_msg << std::endl;
  std::exit(1);
}
