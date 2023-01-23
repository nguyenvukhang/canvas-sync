#include "httpjson.h"
#include <stdexcept>

HttpJson::HttpJson(std::string *token, std::string *base_url)
{
  this->base_url.assign(*base_url);
  this->token.assign(*token);
  // FIXME
  std::cout << "new client with token   [" << this->token << "]" << std::endl;
  std::cout << "new client with base_url[" << this->base_url << "]"
            << std::endl;
}

std::string HttpJson::get(const char *url) const
{
  using namespace httplib;
  Client cli(this->base_url);
  // FIXME
  std::cout << "using base_url[" << this->base_url << "]" << std::endl;
  cli.set_bearer_token_auth(this->token);
  Result res = cli.Get(url);
  if (res->status == 200) {
    return res->body;
  }
  // error handling
  Error err = res.error();
  std::cerr << "HTTP error: " << httplib::to_string(err) << std::endl
            << "Url used: " << this->base_url << url << std::endl;
  throw std::runtime_error("[error] Failed network request.");
};
