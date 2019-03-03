//*************************************************************************
// File session.cc
// Date 24.04.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// Session handler middleware
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <time.h>

#include <cex/session.hpp>
#include <cex/util.hpp>

namespace cex
{

//***************************************************************************
// Middleware sessionHandler
//***************************************************************************

static struct SessionOptions defaultSessionOptions;

MiddlewareFunction sessionHandler(std::shared_ptr<SessionOptions> opts)
{
   // opts is CAPTURED, thus held for the lifetime of the lambda. this is INTENDED, and NOT a leak,
   // so the shared_ptr is not an issue

   MiddlewareFunction res = [opts](Request* req, Response* res, std::function<void()> next)
   {
      SessionOptions* theOpts = opts.get() ? opts.get() : &defaultSessionOptions;
      std::string sessionIDName= theOpts->name;
      const char* cookie = req->get("Cookie");

      if (!sessionIDName.length())
	 sessionIDName= "sessionId";

      if (cookie)
      {
	 // split by ; to iterate all contained cookies

	 std::vector<std::string> splitted(splitString(cookie, ';'));

	 for (std::vector<std::string>::iterator it = splitted.begin(); it != splitted.end(); it++)
	 {
	    std::string cookieName, cookieValue;
	    std::vector<std::string> cookieContents(splitString((*it).c_str(), '='));
	    std::vector<std::string>::iterator cookieIt = cookieContents.begin();

	    // split again by = to get cookie name & value. then try to find 'our' cookie

	    if (cookieContents.size() && cookieIt != cookieContents.end()) 
	    {
	       cookieName= *cookieIt;
	       cookieIt++;

	       if (cookieIt != cookieContents.end())
	       {
		  cookieValue= *cookieIt;

		  if (!cookieName.compare(sessionIDName))
		     req->properties.set(sessionIDName, cookieValue);
	       }
	    }
	 }
      }

      if (!req->properties.has(sessionIDName))
      {
	 // build new cookie when we have no sessionID yet
	 // add all the options according to session-options

	 std::string newSessionId= randomStringHex(32);

         req->properties.set(sessionIDName, newSessionId);

	 std::string setCookie= sessionIDName + "=" + newSessionId;

	 if (theOpts->domain.length())
	    setCookie += "; Domain=" + theOpts->domain;

	 if (theOpts->path.length())
	    setCookie += "; Path=" + theOpts->path;

	 if (theOpts->expires > 0)
	 {
            time_t expDate = time(0) + theOpts->expires;
	    struct tm* timeinfo;
	    char buffer[80];

	    timeinfo= localtime(&expDate);

            if (timeinfo)
            {
               strftime(buffer, 80 , "%a, %d %h %Y %T %Z", timeinfo);

               setCookie += "; Expires=";
               setCookie += (const char*)buffer;
            }
	 }

	 if (theOpts->maxAge > 0)
	 {
	    char secs[80];
	    sprintf(secs, "%ld", theOpts->maxAge);

	    setCookie += "; Max-Age=";
	    setCookie += secs;
	 }

	 if (theOpts->secure)
	    setCookie += "; Secure";

	 if (theOpts->httpOnly)
	    setCookie += "; HttpOnly";

	 if (theOpts->sameSiteStrict)
	    setCookie += "; SameSite=Strict";
	 else if (theOpts->sameSiteLax)
	    setCookie += "; SameSite=Lax";

	 res->set("Set-Cookie", setCookie.c_str());
      }

      next();
   };

   return res;
}

//***************************************************************************
} // namespace cex

