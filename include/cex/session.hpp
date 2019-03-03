//*************************************************************************
// File session.hpp
// Date 24.04.2018
// Copyright (c) 2018-2019 by Patrick Fial
//-------------------------------------------------------------------------
// Session functions
//*************************************************************************

#ifndef __SESSION_HPP__
#define __SESSION_HPP__

/*! \file session.hpp 
  \brief Session middleware function
 
Example:
 ```
   // using default session options

   app.use(cex::sessionHandler());
 ```*/

//***************************************************************************
// includes
//***************************************************************************

#include <string>
#include "core.hpp"

namespace cex
{

//**************************************************************************
// Middlewares
//***************************************************************************
// Session
//***************************************************************************

/*! \struct SessionOptions
  \brief Contains all options for the sessionHandler middleware
 
Example:
```
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
   
   app.use(cex::sessionHandler(opts));
```
will set the following cookie:

```
Set-Cookie: sessionID=D7D1AB0E9B41E9291933C28DB7110A8B6C01B47D13EF82D712676E12AFF97A85; Domain=my.domain.de; Path=/somePath; Expires=Sun, 10 Feb 2019 08:54:31 CET; Max-Age=144; HttpOnly; SameSite=Strict
```
 */

struct SessionOptions
{
   SessionOptions() { name= "sessionId"; secure= false; httpOnly= true; secure= sameSiteStrict= sameSiteLax= false; expires= 0; maxAge= 0; }

   /*! \brief Sets the expires cookie option. Must be a relative time offset in seconds. */
   time_t expires;

   /*! \brief Sets the maxAge cookie option */
   long maxAge;

   /*! \brief Sets the domain cookie option */
   std::string domain;

   /*! \brief sets the path cookie option */
   std::string path;

   /*! \brief sets the name cookie option */
   std::string name;

   /*! \brief sets the secure cookie option */
   bool secure;

   /*! \brief sets the HTTP-only cookie option */
   bool httpOnly;

   /*! \brief sets the same-site (strict) cookie option */
   bool sameSiteStrict;
   
   /*! \brief sets the same-site (lax) cookie option */
   bool sameSiteLax;
};

/*! \public 
  \brief Creates a middleware function which gets/creates session IDs 
 
  Extracts the cookie with the configured name from the request. If no cookie could be found, a new session ID is created and a 
  cookie is attached to the Response object. The session ID is also added to the Request object's property list. The name of the 
  property corresponds to the cookie/session ID name.
 */
MiddlewareFunction sessionHandler(std::shared_ptr<SessionOptions> opts = nullptr);


//***************************************************************************
} // namespace cex

#endif // __SESSION_HPP_
