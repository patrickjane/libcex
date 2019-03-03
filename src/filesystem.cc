//*************************************************************************
// File filesystem.cc
// Date 24.05.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// Filesystem middleware
// Loads content from local filesystem
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <cex/filesystem.hpp>

#include <fstream>
#include <errno.h>
#include <ctype.h>

namespace cex
{

//***************************************************************************
// Middleware filesystem
//***************************************************************************

static struct FilesystemOptions defaultOptions;

MiddlewareFunction filesystem(std::string aPath)
{
   std::shared_ptr<FilesystemOptions> opts(new FilesystemOptions());
   opts.get()->rootPath= aPath;

   return filesystem(opts);
}

MiddlewareFunction filesystem(std::shared_ptr<FilesystemOptions> opts)
{
   // opts is CAPTURED, thus held for the lifetime of the lambda. this is INTENDED, and NOT a leak,
   // so the shared_ptr is not an issue

   if (opts.get() && !opts.get()->rootPath.empty() && opts.get()->rootPath.back() != '/')
      opts.get()->rootPath.push_back('/');

   MiddlewareFunction res = [opts](Request* req, Response* res, std::function<void()> next)
   {
      FilesystemOptions* theOpts = opts.get() ? opts.get() : &defaultOptions;

      if (theOpts->rootPath.empty())
         return next();

      // (1) sanitize incoming URL before creating local FS path
      //     (remove any ../ leading / and any double /)

      std::string url(theOpts->rootPath);
      std::string extension;
      const char* p= req->getUrl();
      const char* urlBeg= p;
      const char* middlewarePath= req->getMiddlewarePath() ? req->getMiddlewarePath() : "";
      int middlewarePathLen= strlen(middlewarePath);
      MimeType type = std::make_pair("text/plain", false);

      if (!p || !strlen(p))
      {
         res->end(404);
         return;
      }

      if (p && middlewarePathLen && !strncmp(p, middlewarePath, middlewarePathLen))
         p += middlewarePathLen;

      while (p && *p == '/')
         p++;

      while (p && *p)
      {
         while (*p && (!strncmp(p, "../", 3) || (*p == '/' && url.back() == '/')))
         {
            if (*p == '/')
               p++;
            else
               p += 3;
         }

         if (!*p)
            break;

         url.push_back(*p);
         p++;
      }

      // (2) determine mime type & correctly set Content-Type header

      p= req->getUrl() + strlen(req->getUrl()) - 1;

      while (p >= urlBeg && *p && isalnum(*p) && *p != '.')
         p--;

      if (*p == '.')
         extension= p+1;

      if (!extension.empty() && Server::getMimeTypes()->count(extension))
      {
         type= (*Server::getMimeTypes())[extension];

         std::string cntType= type.first;

         if (!type.second)
         {
            cntType+= "; charset=";
            cntType+= theOpts->defaultEncoding;
         }

         res->set("Content-Type", cntType.c_str());
      }
      else
      {
         std::string cntType= "text/plain; charset=";
         cntType += theOpts->defaultEncoding;

         res->set("Content-Type", cntType.c_str());
      }

      // (3) open the file using ifstream

      std::ifstream file(url.c_str(), std::ios::in|std::ios::binary);

      // (4a) respond 404 if file could not be found

      if (!file.is_open() || !file.good())
      {
         res->end(404);
         return;
      }

      // (4b) reply with binary data using the stream interface (send chunks)

      res->stream(200, &file);
      file.close();
   };

   return res;
}

//***************************************************************************
} // namespace cex

