#include "canvas_api.h"
#include "httpclient.h"
#include <gtest/gtest.h>

class FakeServer : public HttpClient {
private:
  std::string token;
  std::string base_url;

public:
  std::string get(const char *url) const override {
    // Profile of the canvas user.
    // https://canvas.instructure.com/doc/api/users.html#Profile
    if (strcmp("/api/v1/users/self/profile", url) == 0) {
      return R"#({
        "id": 12345,
        "name": "MITSUHA MIYAMIZU",
        "short_name": "MITSUHA MIYAMIZU",
        "sortable_name": "MITSUHA MIYAMIZU",
        "avatar_url": "https://canvas.nus.edu.sg/images/thumbnails/1234/adsf",
        "title": null,
        "bio": "",
        "primary_email": "e0000000@u.nus.edu",
        "login_id": "e0000000",
        "integration_id": "A0000000A",
        "time_zone": "Asia/Singapore",
        "locale": null,
        "effective_locale": "en-GB",
        "calendar": { "ics": "https://canvas.nus.edu.sg/feeds/calendars/user_1234.ics" },
        "lti_user_id": "1234",
        "k5_user": false
      })#";
    }
    // no routes match the url.
    return "[fakeserver] 404 route not found";
  }
};

class CanvasApiTest : public ::testing::Test {
protected:
  void SetUp() override {}
  // convenience routine to create a fake API.
  CanvasApi *api() { return new CanvasApi(new FakeServer()); }
};

TEST_F(CanvasApiTest, Profile) {
  Profile p = api()->profile();
  EXPECT_EQ(p.id, 12345);
  EXPECT_EQ(p.integration_id, "A0000000A");
  EXPECT_EQ(p.name, "MITSUHA MIYAMIZU");
  EXPECT_EQ(p.primary_email, "e0000000@u.nus.edu");
  EXPECT_EQ(p.login_id, "e0000000");
}
