#include "httpjson.h"
#include <stdexcept>

HttpJson::HttpJson(std::string &token, std::string &base_url)
{
  this->base_url.assign(base_url);
  this->token.assign(token);
}

[[noreturn]] void http_error(httplib::Error err, std::string url)
{
  std::cerr << "HTTP error: " << httplib::to_string(err) << std::endl
            << "Url used: " << url << std::endl;
  throw std::runtime_error("[error] Failed network request.");
}

std::string HttpJson::get(const std::string &url) const
{
  using namespace httplib;
  Client cli(this->base_url);
  cli.set_bearer_token_auth(this->token);
  Result res = cli.Get(url);
  if (res && res->status == 200 || res.error() == httplib::Error::Success) {
    return res->body;
  }
  http_error(res.error(), this->base_url + url);
};

void HttpJson::get(const std::string &url, ContentReceiver rec) const
{
  using namespace httplib;
  Client cli(this->base_url);
  cli.set_bearer_token_auth(this->token);
  cli.set_follow_location(true);
  Result res = cli.Get(url, rec);
  if (res->status == 200 || res.error() == httplib::Error::Success) {
    return;
  }
  http_error(res.error(), this->base_url + url);
};
