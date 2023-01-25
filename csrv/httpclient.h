#ifndef CANVAS_SYNC_HTTPCLIENT_H
#define CANVAS_SYNC_HTTPCLIENT_H

#include <functional>
#include <string>

class HttpClient
{
public:
  using ContentReceiver =
      std::function<bool(const char *data, size_t data_length)>;
  virtual std::string get(const std::string &path) const = 0;
  virtual void get(const std::string &path,
                   ContentReceiver content_receiver) const = 0;
};

#endif
