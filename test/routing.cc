//*************************************************************************
// File routing.cc
// Date 05.09.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// cex Library routing functionality testcases
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <bandit/bandit.h>
#include <httplib.h>
#include <cex.hpp>

using namespace snowhouse;
using namespace bandit;

//***************************************************************************
// testcase definitions
//***************************************************************************

go_bandit([]() 
{
   //************************************************************************
   // Routing functions
   //************************************************************************

   describe("Statuscodes and path based routes", []() 
   {
      int port= 15555;
      const char* host= "127.0.0.1";
      const char* payload= "<h1>It works!</h1>";

      cex::Server app;
      httplib::Client cli(host, port);

      app.use("/notfound", [&payload](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(404);
      });

      app.use("/servererror", [&payload](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(500);
      });

      app.use("^/$", [&payload](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(payload, 200);
      }, cex::Middleware::fMatchRegex);

      app.use("something", [&payload](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(payload, 200);
      }, cex::Middleware::fMatchContain);

      app.use("/must/match/exactly", [&payload](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(payload, 200);
      }, cex::Middleware::fMatchCompare);


      app.use([&payload](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(400);
      });

      app.listen(host, port, 0 /* don't block */);

      //*********************************************************************
      // testcases
      //*********************************************************************

      it("should return 200 for / (regexp-match vs '^/$')", [&]() 
      {
         auto res = cli.Get("/");

         AssertThat(res->status, Equals(200));
         AssertThat(res->body.c_str(), Equals(payload));
      });


      it("should return 200 for /some/something/path (contain-match vs 'something')", [&]() 
      {
         auto res = cli.Get("/some/something/path");

         AssertThat(res->status, Equals(200));
         AssertThat(res->body.c_str(), Equals(payload));
      });

      it("should return 200 for /must/match/exactly (compare-match vs '/must/match/exactly')", [&]() 
      {
         auto res = cli.Get("/must/match/exactly");

         AssertThat(res->status, Equals(200));
         AssertThat(res->body.c_str(), Equals(payload));
      });


      it("should return 400 for /test", [&]() 
      {
         auto res = cli.Get("/test");

         AssertThat(res->status, Equals(400));
      });

      it("should return 404 for /notfound middleware", [&]() 
      {
         auto res = cli.Get("/notfound");

         AssertThat(res->status, Equals(404));
         AssertThat(res->body.c_str(), Equals(""));
      });

      it("should return 500 for /servererror middleware", [&]() 
      {
         auto res = cli.Get("/servererror");

         AssertThat(res->status, Equals(500));
         AssertThat(res->body.c_str(), Equals(""));
      });
   });

   //************************************************************************
   // Method based routing
   //************************************************************************

   describe("Method based routing", []() 
   {
      int port= 15555;
      const char* host= "127.0.0.1";

      cex::Server app;
      httplib::Client cli(host, port);

      cex::Server::libraryInit();

      app.get("/onlyget", [&](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.post("/onlypost", [&](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.put("/onlyput", [&](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.head("/onlyhead", [&](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.del("/onlydelete", [&](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.options("/onlyoptions", [&](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.trace("/onlytrace", [&](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.patch("/onlypatch", [&](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(200);
      });

      app.use([&](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(500);
      });

      app.listen(host, port, 0 /* don't block */);

      //*********************************************************************
      // testcases
      //*********************************************************************

      it("should /onlyget routing should only reply to GET method", [&]() 
      {
         auto res0 = cli.Get("/onlyget");
         auto res1 = cli.Post("/onlyget", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res2 = cli.Put("/onlyget", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res3 = cli.Head("/onlyget");
         auto res4 = cli.Delete("/onlyget");
         auto res5 = cli.Options("/onlyget");
         auto res6 = cli.Trace("/onlyget");
         auto res7 = cli.Patch("/onlyget");

         AssertThat(res0->status, Equals(200));
         AssertThat(res1->status, Equals(500));
         AssertThat(res2->status, Equals(500));
         AssertThat(res3->status, Equals(500));
         AssertThat(res4->status, Equals(500));
         AssertThat(res5->status, Equals(500));
         AssertThat(res6->status, Equals(500));
         AssertThat(res7->status, Equals(500));
      });

      it("should /onlypost routing should only reply to POST method", [&]() 
      {
         auto res0 = cli.Get("/onlypost");
         auto res1 = cli.Post("/onlypost", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res2 = cli.Put("/onlypost", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res3 = cli.Head("/onlypost");
         auto res4 = cli.Delete("/onlypost");
         auto res5 = cli.Options("/onlypost");
         auto res6 = cli.Trace("/onlypost");
         auto res7 = cli.Patch("/onlypost");

         AssertThat(res0->status, Equals(500));
         AssertThat(res1->status, Equals(200));
         AssertThat(res2->status, Equals(500));
         AssertThat(res3->status, Equals(500));
         AssertThat(res4->status, Equals(500));
         AssertThat(res5->status, Equals(500));
         AssertThat(res6->status, Equals(500));
         AssertThat(res7->status, Equals(500));
      });

      it("should /onlyput routing should only reply to PUT method", [&]() 
      {
         auto res0 = cli.Get("/onlyput");
         auto res1 = cli.Post("/onlyput", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res2 = cli.Put("/onlyput", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res3 = cli.Head("/onlyput");
         auto res4 = cli.Delete("/onlyput");
         auto res5 = cli.Options("/onlyput");
         auto res6 = cli.Trace("/onlyput");
         auto res7 = cli.Patch("/onlyput");

         AssertThat(res0->status, Equals(500));
         AssertThat(res1->status, Equals(500));
         AssertThat(res2->status, Equals(200));
         AssertThat(res3->status, Equals(500));
         AssertThat(res4->status, Equals(500));
         AssertThat(res5->status, Equals(500));
         AssertThat(res6->status, Equals(500));
         AssertThat(res7->status, Equals(500));
      });

      it("should /onlyhead routing should only reply to HEAD method", [&]() 
      {
         auto res0 = cli.Get("/onlyhead");
         auto res1 = cli.Post("/onlyhead", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res2 = cli.Put("/onlyhead", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res3 = cli.Head("/onlyhead");
         auto res4 = cli.Delete("/onlyhead");
         auto res5 = cli.Options("/onlyhead");
         auto res6 = cli.Trace("/onlyhead");
         auto res7 = cli.Patch("/onlyhead");

         AssertThat(res0->status, Equals(500));
         AssertThat(res1->status, Equals(500));
         AssertThat(res2->status, Equals(500));
         AssertThat(res3->status, Equals(200));
         AssertThat(res4->status, Equals(500));
         AssertThat(res5->status, Equals(500));
         AssertThat(res6->status, Equals(500));
         AssertThat(res7->status, Equals(500));
      });

      it("should /onlydelete routing should only reply to DELETE method", [&]() 
      {
         auto res0 = cli.Get("/onlydelete");
         auto res1 = cli.Post("/onlydelete", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res2 = cli.Put("/onlydelete", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res3 = cli.Head("/onlydelete");
         auto res4 = cli.Delete("/onlydelete");
         auto res5 = cli.Options("/onlydelete");
         auto res6 = cli.Trace("/onlydelete");
         auto res7 = cli.Patch("/onlydelete");

         AssertThat(res0->status, Equals(500));
         AssertThat(res1->status, Equals(500));
         AssertThat(res2->status, Equals(500));
         AssertThat(res3->status, Equals(500));
         AssertThat(res4->status, Equals(200));
         AssertThat(res5->status, Equals(500));
         AssertThat(res6->status, Equals(500));
         AssertThat(res7->status, Equals(500));
      });

      it("should /onlyoptions routing should only reply to OPTIONS method", [&]() 
      {
         auto res0 = cli.Get("/onlyoptions");
         auto res1 = cli.Post("/onlyoptions", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res2 = cli.Put("/onlyoptions", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res3 = cli.Head("/onlyoptions");
         auto res4 = cli.Delete("/onlyoptions");
         auto res5 = cli.Options("/onlyoptions");
         auto res6 = cli.Trace("/onlyoptions");
         auto res7 = cli.Patch("/onlyoptions");

         AssertThat(res0->status, Equals(500));
         AssertThat(res1->status, Equals(500));
         AssertThat(res2->status, Equals(500));
         AssertThat(res3->status, Equals(500));
         AssertThat(res4->status, Equals(500));
         AssertThat(res5->status, Equals(200));
         AssertThat(res6->status, Equals(500));
         AssertThat(res7->status, Equals(500));
      });

      it("should /onlytrace routing should only reply to TRACE method", [&]() 
      {
         auto res0 = cli.Get("/onlytrace");
         auto res1 = cli.Post("/onlytrace", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res2 = cli.Put("/onlytrace", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res3 = cli.Head("/onlytrace");
         auto res4 = cli.Delete("/onlytrace");
         auto res5 = cli.Options("/onlytrace");
         auto res6 = cli.Trace("/onlytrace");
         auto res7 = cli.Patch("/onlytrace");

         AssertThat(res0->status, Equals(500));
         AssertThat(res1->status, Equals(500));
         AssertThat(res2->status, Equals(500));
         AssertThat(res3->status, Equals(500));
         AssertThat(res4->status, Equals(500));
         AssertThat(res5->status, Equals(500));
         AssertThat(res6->status, Equals(200));
         AssertThat(res7->status, Equals(500));
      });

      it("should /onlypatch routing should only reply to PATCH method", [&]() 
      {
         auto res0 = cli.Get("/onlypatch");
         auto res1 = cli.Post("/onlypatch", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res2 = cli.Put("/onlypatch", "name=john1&note=coder", "application/x-www-form-urlencoded");
         auto res3 = cli.Head("/onlypatch");
         auto res4 = cli.Delete("/onlypatch");
         auto res5 = cli.Options("/onlypatch");
         auto res6 = cli.Trace("/onlypatch");
         auto res7 = cli.Patch("/onlypatch");

         AssertThat(res0->status, Equals(500));
         AssertThat(res1->status, Equals(500));
         AssertThat(res2->status, Equals(500));
         AssertThat(res3->status, Equals(500));
         AssertThat(res4->status, Equals(500));
         AssertThat(res5->status, Equals(500));
         AssertThat(res6->status, Equals(500));
         AssertThat(res7->status, Equals(200));
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
