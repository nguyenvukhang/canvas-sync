#ifndef CANVAS_SYNC_DEBUG_H
#define CANVAS_SYNC_DEBUG_H
#include <iostream>

namespace dbg {
inline void good() {
  std::cerr << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> GOOD" << std::endl;
}
inline void bad() {
  std::cerr << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> BAD" << std::endl;
}
} // namespace dbg

#endif
