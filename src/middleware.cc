//*************************************************************************
// File middleware.cc
// Date 24.04.2018 - #1
// Copyright (c) 2018-2019 by Patrick Fial
//-------------------------------------------------------------------------
// cex Library Middleware class implementation
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <iostream>

#include <cex/core.hpp>
#include <cex/util.hpp>

namespace cex
{

//***************************************************************************
// class Middleware
//***************************************************************************
// ctor/dtor
//***************************************************************************

Middleware::Middleware(const char* aPath, MiddlewareFunction func, int aMethod, int aFlags)
   : func(std::move(func)), rep(aPath ? aPath : ".*", std::regex::optimize),
     method(aMethod), path(aPath ? aPath : ""), flags(aFlags)
{
   if (!aPath)
      flags= flags & ~fMatching;

   type= tpStandard;
}

Middleware::Middleware(const char* aPath, UploadFunction func, int aMethod, int aFlags)
   : uploadFunc(std::move(func)), rep(aPath ? aPath : ".*", std::regex::optimize),
     method(aMethod), path(aPath ? aPath : ""), flags(aFlags)
{
   if (!aPath)
      flags= flags & ~fMatching;

   type= tpUpload;
}

//***************************************************************************
// match
//***************************************************************************

bool Middleware::match(Request* req)
{
   if (method != na && method != req->evhtp_method)
      return false;

   if (!(flags & fMatching))
      return true;

   if (!req || !req->getUrl())
      return false;

   // plain strcmp, faster than regex

   if (flags & fMatchCompare)
      return !strcmp(notNull(req->getUrl()), path.c_str()) ? true : false;

   // plain strstr, faster than regex

   if (flags & fMatchContain)
      return strstr(notNull(req->getUrl()), path.c_str()) ? true : false;

   // full regular expression matching

   std::cmatch m;
   std::regex_search(req->getUrl(), m, rep);

   return m.size() > 0;
}

//***************************************************************************
} // namespace cex

