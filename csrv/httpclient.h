#ifndef CANVAS_SYNC_HTTPCLIENT_H
#define CANVAS_SYNC_HTTPCLIENT_H

#include <string>

class HttpClient {
public:
  virtual std::string get(const char *url) const = 0;
};

#endif
