//*************************************************************************
// File mw_session.cc
// Date 07.02.2019 - #1
// Copyright (c) 2019-2019 by Patrick Fial
//-------------------------------------------------------------------------
// cex Library session middleware testcases
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <bandit/bandit.h>
#include <httplib.h>
#include <cex.hpp>
#include <cex/session.hpp>

#include <time.h>

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

      std::shared_ptr<cex::SessionOptions> opts(new cex::SessionOptions());

      opts.get()->expires = 60*60*24*3;
      opts.get()->maxAge= 144; 
      opts.get()->domain= "my.domain.de"; 
      opts.get()->path= "/somePath"; 
      opts.get()->name= "sessionID"; 
      opts.get()->secure= false; 
      opts.get()->httpOnly=true; 
      opts.get()->sameSiteLax= true; 
      opts.get()->sameSiteStrict= true;
 
      app.use("/withsession", cex::sessionHandler(opts));
      app.use("/withsession",  [](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.use("/nosession",  [](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.listen(host, port, 0 /* don't block */);

      //*********************************************************************
      // testcases
      //*********************************************************************

      it("should set a Set-Cookie header with all cookie settings for GET /withsession", [&]() 
      {
         auto res = cli.Get("/withsession");
         std::string cv = res->get_header_value("Set-Cookie");
         char timeBuf[200];

         time_t rawtime = time(0) + 60*60*24*3;
         struct tm* timeinfo = localtime(&rawtime);

         strftime(timeBuf, 200, "Expires=%a, %d %h %Y", timeinfo);

         AssertThat(res->status, Equals(200));
         AssertThat(res->has_header("Set-Cookie"), Equals(true));
         AssertThat(cv, Is().Not().OfLength(0));
         AssertThat(cv, Is().StartingWith("sessionID="));
         AssertThat(cv, Is().Containing("HttpOnly"));
         AssertThat(cv, Is().Containing("Max-Age=144"));
         AssertThat(cv, Is().Containing(std::string(timeBuf)));
         AssertThat(cv, Is().Containing("Path=/somePath"));
         AssertThat(cv, Is().Containing("Domain=my.domain.de"));
         AssertThat(cv, Is().Containing("SameSite=Strict"));
      });

      it("should NOT set a Set-Cookie header for GET /nosession", [&]() 
      {
         auto res = cli.Get("/nosession");

         AssertThat(res->status, Equals(200));
         AssertThat(res->has_header("Set-Cookie"), Equals(false));
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

