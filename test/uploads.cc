//*************************************************************************
// File uploads.cc
// Date 08.10.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// cex Library (file-)upload functionality testcases
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <bandit/bandit.h>
#include <httplib.h>
#include <cex.hpp>

#ifdef CEX_WITH_SSL
#  include <openssl/md5.h>
#endif

#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>

using namespace snowhouse;
using namespace bandit;

void getMD5Sum(const unsigned char* data, size_t len, char* target);

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
      std::vector<char> buffer;
      char targetHash[32+1]; *targetHash= 0;
      char md5Hash[32+1]; *md5Hash= 0;
      memset(md5Hash, 0, sizeof(md5Hash));

      // init libcex

      cex::Server app;
      httplib::Client cli(host, port);

      app.uploads("/uploads", [&buffer](cex::Request* req, const char* data, size_t len) 
      {
         size_t oldSize= buffer.size();

         buffer.resize(buffer.size() + len);

         if (data && len)
            memcpy(buffer.data() + oldSize, data, len);
      });
   
      app.post([&buffer, &md5Hash](cex::Request* req, cex::Response* res, std::function<void()> next)
      {
         getMD5Sum((unsigned const char*)buffer.data(), buffer.size(), md5Hash);
         res->end(md5Hash, 200);
      });

      app.listen(host, port, 0 /* don't block */);

      // read in file contents

      size_t fileSize= 0;
      char* buf= 0;
      int fd= 0;

      fd= ::open("testdata/uploads/test.jpg", O_RDONLY);

      AssertThat(fd, Is().GreaterThan(0));

      fileSize= lseek(fd, 0, SEEK_END);
      
      lseek(fd, 0, SEEK_SET);

      buf= (char*)calloc(fileSize, 1);

      ::read(fd, buf, fileSize);
      ::close(fd);

      getMD5Sum((unsigned const char*)buf, fileSize, targetHash);

      //*********************************************************************
      // testcases
      //*********************************************************************

#ifdef CEX_WITH_SSL
      it("should return 200 and correct MD5 for uploaded PNG file", [&]() 
      {
         std::string contents(buf, fileSize);
         auto res = cli.Post("/uploads/test.jpg", contents, "image/jpeg");
         const char* h= targetHash;

         AssertThat(res->status, Equals(200));
         AssertThat(res->body.c_str(), Equals(h));
      });
#endif
   });
});

//***************************************************************************
// helpers
//***************************************************************************

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

