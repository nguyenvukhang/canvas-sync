#ifndef HTTPJSON_H
#define HTTPJSON_H

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "httpclient.h"
#include "httplib.h"

class HttpJson : public HttpClient
{
  std::string token;
  std::string base_url;

public:
  HttpJson(std::string *token, std::string *base_url);
  std::string get(const std::string &path) const override;
  void get(const std::string &path, ContentReceiver) const override;
};

#endif
