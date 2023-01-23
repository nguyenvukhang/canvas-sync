#ifndef HTTPJSON_H
#define HTTPJSON_H

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "httpclient.h"
#include "httplib.h"

class HttpJson : public HttpClient {
private:
  std::string token;
  std::string base_url;

public:
  HttpJson(std::string *token, std::string *base_url);
  std::string get(const char *url) const override;
};

#endif
