//*************************************************************************
// File basicauth.hpp
// Date 10.09.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// HTTP basic auth functions
// Middleware that sets username/password from HTTP basic authentication header
//*************************************************************************

#ifndef __BASICAUTH_HPP__
#define __BASICAUTH_HPP__

/*! \file basicauth.hpp 
  \brief Basic auth middleware function

  Tries to extract username and password information from the `Authorization` HTTP header. Only allows
  to retrieve `Basic` authentication information.

  Upon success (e.g. a `Authorization` header is present and its contents could be extracted) stores the 
  values in the Request object's properties `basicUsername` and `basicPassword`.
*/

//***************************************************************************
// includes
//***************************************************************************

#include <string>
#include <core.hpp>

namespace cex
{

//**************************************************************************
// Middlewares
//***************************************************************************
// basicAuth
//***************************************************************************

/*! \public 
  \brief Creates a middleware that extracts username and password information from the `Authorization` HTTP header
 */

MiddlewareFunction basicAuth();

//***************************************************************************
} // namespace cex

#endif // __BASICAUTH_HPP_
