//*************************************************************************
// File basicauth.cc
// Date 10.09.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// HTTP basic auth functions
// Middleware that sets username/password from HTTP basic authentication header
//*************************************************************************
//***************************************************************************
// includes
//***************************************************************************

#include <cex/basicauth.hpp>
#include <cex/util.hpp>   // splitString

namespace cex
{

static const int B64index[256] = 
{ 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
   56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
   7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
   0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 
};

//***************************************************************************
// Middleware basicAuth
//***************************************************************************

MiddlewareFunction basicAuth()
{
   MiddlewareFunction res = [](Request* req, Response* res, std::function<void()> next)
   {
      const char* authenticationHeader= req->get("Authorization");

      if (!authenticationHeader || strncmp(authenticationHeader, "Basic", 5) || strlen(authenticationHeader) < 7)
      {
         next();
         return;
      }

      // decode base64 value (credits: https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c/41094722#41094722)

      size_t len= strlen(authenticationHeader + 6);
      unsigned char* p = (unsigned char*)authenticationHeader + 6;
      int pad = len > 0 && (len % 4 || p[len - 1] == '=');
      const size_t L = ((len + 3) / 4 - pad) * 4;
      std::string str(L / 4 * 3 + pad, '\0');

      for (size_t i = 0, j = 0; i < L; i += 4)
      {
         int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
         str[j++] = n >> 16;
         str[j++] = n >> 8 & 0xFF;
         str[j++] = n & 0xFF;
      }

      if (pad)
      {
         int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
         str[str.size() - 1] = n >> 16;

         if (len > L + 2 && p[L + 2] != '=')
         {
            n |= B64index[p[L + 2]] << 6;
            str.push_back(n >> 8 & 0xFF);
         }
      }

      // split the decoded value by ':' and save it in request plist

      if (str.length())
      {
         std::vector<std::string> splitted= splitString(str.c_str(), ':', 0);

         if (splitted.size())
         {
            req->properties.set("basicUsername", splitted[0]);

            if (splitted.size() > 1)
               req->properties.set("basicPassword", splitted[1]);
         }
      }

      next();
   };

   return res;
}

//***************************************************************************
} // namespace cex

