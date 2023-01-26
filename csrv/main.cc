#include "csrv.h"
#include <stdlib.h>

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "httplib.h"

std::string get_env(std::string var)
{
  const char *v = std::getenv(var.c_str());
  return v ? v : "(no env var found for $" + var + ')';
}

/**
 * From Qt frontend, we need to specify an equivalent class
 * that enables a GET request that supports the HTTPS protocol
 */
class RealClient : public HttpClient
{
private:
  httplib::Client cli;
  const std::string base_url;

public:
  RealClient(const std::string base_url, const std::string token)
      : base_url(base_url), cli(base_url)
  {
    cli.set_bearer_token_auth(token);

    // keep in mind this is relative to where you, the user, executes the
    // binary. this is Not relative to where the binary is stored.
    cli.set_ca_cert_path("./cert.pem");
  };
  std::string get(const std::string &url) override
  {
    httplib::Result res = this->cli.Get(url);
    if (res) {
      return this->cli.Get(url)->body;
    }
    std::cerr << "HTTP error: " << httplib::to_string(res.error()) << std::endl
              << "Url used: " << url << std::endl;
    return "[HTTP Request Error]";
  };
};

#define POINT(key, val) std::cout << "  * " << key << ": " << val << std::endl;

// for debugging purposes, contained within this library only.
int main()
{
  std::cout << "------ [CSRV (Canvas Server) Debug Run] ------" << std::endl;
  std::cout << "[token]: " << get_env("CANVAS_TOKEN") << std::endl;
  RealClient rc("https://canvas.nus.edu.sg", get_env("CANVAS_TOKEN"));
  canvas::Server server(&rc, "");
  auto p = server.get_profile();
  std::cout << "----- Get Profile -----" << std::endl;
  POINT("name", p.name);
  POINT("id  ", p.integration_id);
  std::cout << "loading...." << std::endl;
  server.load();
  std::vector<Update> u;
  debug(server.get_tree());

  // canvas::Server s("", "https://canvas.nus.edu.sg");
  std::cout << "------ [CSRV (Canvas Server) End Nicely] ------" << std::endl;
  return 0;
}
