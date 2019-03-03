//*************************************************************************
// File mw_security.cc
// Date 10.02.2019
// Copyright (c) 2019-2019 by Patrick Fial
//-------------------------------------------------------------------------
// cex Library security middleware testcases
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <bandit/bandit.h>
#include <httplib.h>
#include <cex.hpp>
#include <cex/security.hpp>

using namespace snowhouse;
using namespace bandit;

//***************************************************************************
// testcase definitions
//***************************************************************************

go_bandit([]() 
{
   //************************************************************************
   // Uploads functions
   //************************************************************************

   describe("Statuscodes and path based routes", []() 
   {
      int port= 15555;
      const char* host= "127.0.0.1";

      // init libcex

      cex::Server app;
      httplib::Client cli(host, port);

      std::shared_ptr<cex::SecurityOptions> opts(new cex::SecurityOptions());

      opts.get()->hpkpKeys.push_back("someKey");
      opts.get()->hpkpKeys.push_back("someKey2");
      opts.get()->hpkpKeys.push_back("someKey3");
      opts.get()->hpkpMaxAge= 183400;
      opts.get()->xFrameAllow= cex::xfFrom;
      opts.get()->xFrameFrom= "my.domain.de";
      opts.get()->stsMaxAge= 183400;
      opts.get()->stsPreload= true;
      opts.get()->ieNoOpen= cex::no;
      opts.get()->noDNSPrefetch= cex::no;
      opts.get()->referrer= cex::refOriginWhenCrossOrigin;

      app.use("/withsecurity", cex::securityHeaders(opts));
      app.use("/withsecurity",  [](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.use("/nosecurity",  [](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.listen(host, port, 0 /* don't block */);

      //*********************************************************************
      // testcases
      //*********************************************************************

      it("should set security headers for GET /withsecurity", [&]() 
      {
         auto res = cli.Get("/withsecurity");

         AssertThat(res->status, Equals(200));
         AssertThat(res->get_header_value("X-DNS-Prefetch-Control"), Equals("on"));
         AssertThat(res->get_header_value("X-Frame-Options"), Equals("ALLOW-FROM my.domain.de"));
         AssertThat(res->get_header_value("Public-Key-Pins"), Equals("pin-sha256=\"someKey\"; pin-sha256=\"someKey2\"; pin-sha256=\"someKey3\"; max-age=183400; includeSubdomains"));
         AssertThat(res->get_header_value("Strict-Transport-Security"), Equals("max-age=183400; preload"));
         AssertThat(res->get_header_value("Cache-Control"), Equals("no-store, no-cache, must-revalidate, proxy-revalidate"));
         AssertThat(res->get_header_value("Pragma"), Equals("no-cache"));
         AssertThat(res->get_header_value("Expires"), Equals("0"));
         AssertThat(res->get_header_value("X-Content-Type-Options"), Equals("nosniff"));
         AssertThat(res->get_header_value("Referrer-Policy"), Equals("origin-when-cross-origin"));
         AssertThat(res->get_header_value("X-XSS-Protection"), Equals("1; mode=block"));
      });

      it("should NOT set security headers for GET /nosecurity", [&]() 
      {
         auto res = cli.Get("/nosecurity");

         AssertThat(res->status, Equals(200));
         AssertThat(res->has_header("X-DNS-Prefetch-Control"), Equals(false));
         AssertThat(res->has_header("X-Frame-Options"), Equals(false));
         AssertThat(res->has_header("Public-Key-Pins"), Equals(false));
         AssertThat(res->has_header("Strict-Transport-Security"), Equals(false));
         AssertThat(res->has_header("Cache-Control"), Equals(false));
         AssertThat(res->has_header("Pragma"), Equals(false));
         AssertThat(res->has_header("Expires"), Equals(false));
         AssertThat(res->has_header("X-Content-Type-Options"), Equals(false));
         AssertThat(res->has_header("Referrer-Policy"), Equals(false));
         AssertThat(res->has_header("X-XSS-Protection"), Equals(false));
      });
   });
});

//***************************************************************************
// main
//***************************************************************************

int main(int argc, char* argv[]) 
{
   return bandit::run(argc, argv);
}

