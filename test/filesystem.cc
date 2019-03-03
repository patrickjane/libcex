//*************************************************************************
// File filesystem.cc
// Date 05.09.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// cex Library filesystem functionality testcases
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#ifdef CEX_WITH_ZLIB
#  define CPPHTTPLIB_ZLIB_SUPPORT
#endif

#include <bandit/bandit.h>
#include <httplib.h>
#include <cex.hpp>
#include <cex/filesystem.hpp>

#ifdef CEX_WITH_SSL
#  include <openssl/md5.h>
#endif

using namespace snowhouse;
using namespace bandit;

void AssertMD5(std::string& body, const char* aMD5);
void getMD5Sum(const unsigned char* data, size_t len, char* target);

//***************************************************************************
// testcase definitions
//***************************************************************************

go_bandit([]() 
{
   //************************************************************************
   // filesystem middleware testcases
   //************************************************************************

   describe("Filesystem middleware testcases", []() 
   {
      int port= 15555;
      const char* host= "127.0.0.1";
      const char* payload= "<h1>It works!</h1>\n";

      cex::Server app;
      httplib::Client cli(host, port);

      std::shared_ptr<cex::FilesystemOptions> fsOpts(new cex::FilesystemOptions());

      fsOpts.get()->rootPath= "testdata/filesystem";

      // add middlewares to enable compression on per-request base

      app.use("/gzipContent", [](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->setFlags(res->getFlags() | cex::Response::fCompressGZip);
         next();
      });

      app.use("/deflateContent", [](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->setFlags(res->getFlags() | cex::Response::fCompressDeflate);
         next();
      });

      // different routings/endpoints but same source folder on filesystem

      app.use("/gzipContent", cex::filesystem(fsOpts));
      app.use("/deflateContent", cex::filesystem(fsOpts));
      app.use("/content", cex::filesystem(fsOpts));

      app.use(cex::filesystem(fsOpts));

      app.use([&payload](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         res->end(400);
      });

      app.listen(host, port, 0 /* don't block */);

      //*********************************************************************
      // testcases
      //*********************************************************************

      it("should correctly return (text) contents of /content/testdata1.txt", [&]() 
      {
         auto res = cli.Get("/content/testdata1.txt");

         AssertThat(res->status, Equals(200));
         AssertThat(res->body.c_str(), Equals(payload));
         AssertThat(res->has_header("Content-Encoding"), Equals(false));
         AssertThat(res->get_header_value("Content-Type"), Equals(std::string("text/plain; charset=utf-8")));
      });

#ifdef CEX_WITH_SSL
      it("should correctly return (binary) contents of /content/testdata2.bin", [&]() 
      {
         auto res = cli.Get("/content/testdata2.bin");

         AssertThat(res->status, Equals(200));
         AssertMD5(res->body, "24b73bf87c2f7cda464b9750c86364b8");
         AssertThat(res->has_header("Content-Encoding"), Equals(false));
         AssertThat(res->get_header_value("Content-Type"), Equals(std::string("application/octet-stream")));
      });
#endif

      it("should answer non existing filepaths with 404 (/content/does/not/exist)", [&]() 
      {
         auto res = cli.Get("/content/does/not/exist");

         AssertThat(res->status, Equals(404));
         AssertThat(res->body.size(), Equals(0));
         AssertThat(res->has_header("Content-Encoding"), Equals(false));
      });

      it("should not allow absolute file paths to access files outside the middleware root path (/bin/sh)", [&]() 
      {
         auto res = cli.Get("/bin/sh");

         AssertThat(res->status, Equals(404));
         AssertThat(res->body.size(), Equals(0));
         AssertThat(res->has_header("Content-Encoding"), Equals(false));
      });

#ifdef CEX_WITH_ZLIB
      it("should GZIP compress text contents of /gzipContent/testdata1.txt", [&]() 
      {
         auto res = cli.Get("/gzipContent/testdata1.txt");

         AssertThat(res->status, Equals(200));
         AssertThat(res->body.c_str(), Equals(payload));
         AssertThat(res->has_header("Content-Encoding"), Equals(true));
         AssertThat(res->get_header_value("Content-Encoding"), Equals(std::string("gzip")));
         AssertThat(res->get_header_value("Content-Type"), Equals(std::string("text/plain; charset=utf-8")));
      });

      it("should GZIP compress binary contents of /gzipContent/testdata2.bin", [&]() 
      {
         auto res = cli.Get("/gzipContent/testdata2.bin");

         AssertThat(res->status, Equals(200));
         AssertMD5(res->body, "24b73bf87c2f7cda464b9750c86364b8");
         AssertThat(res->has_header("Content-Encoding"), Equals(true));
         AssertThat(res->get_header_value("Content-Encoding"), Equals(std::string("gzip")));
         AssertThat(res->get_header_value("Content-Type"), Equals(std::string("application/octet-stream")));
      });
#endif
      // cannot be tested because cpp-http-lib does not support deflate compression

//      it("should deflate compress /deflateContent/testdata1.txt", [&]() 
//      {
//         auto res = cli.Get("/deflateContent/testdata1.txt");
//
//         AssertThat(res->status, Equals(200));
//         AssertThat(res->body.c_str(), Equals(payload));
//         AssertThat(res->has_header("Content-Encoding"), Equals(true));
//         AssertThat(res->get_header_value("Content-Encoding"), Equals(std::string("gzip")));
//      });
   });
});

//***************************************************************************
// helpers
//***************************************************************************

void AssertMD5(std::string& body, const char* aMD5)
{
   char md5Hash[32+1];
   memset(md5Hash, 0, sizeof(md5Hash));

   getMD5Sum((const unsigned char*)body.data(), body.size(), md5Hash);

   AssertThat(md5Hash, Equals(aMD5));
}

void getMD5Sum(const unsigned char* data, size_t len, char* target)
{
#ifdef CEX_WITH_SSL
   char* p= target;
   unsigned char result[MD5_DIGEST_LENGTH]; *result= 0;

   if (!data || !target)
      return;

   MD5(data, len, result);

   for (int i= 0; i < MD5_DIGEST_LENGTH; i++)
      p += sprintf(p, "%02x", result[i]);
#endif
}

//***************************************************************************
// main
//***************************************************************************

int main(int argc, char* argv[]) 
{
   return bandit::run(argc, argv);
}

