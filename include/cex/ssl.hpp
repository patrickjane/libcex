//*************************************************************************
// File ssl.hpp
// Date 16.05.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// SSL definitions/helpers
//*************************************************************************

#ifndef __SSL_HPP__
#define __SSL_HPP__

/*! \file ssl.hpp 
  \brief Contains SSL related classes and utilities */
 
//***************************************************************************
// includes
//***************************************************************************

#include <string>
#include <map>

namespace cex
{

//***************************************************************************
// definitions
//***************************************************************************

/*! \public
  \struct CertificateInfo
  \brief Contains the parsed values of a SSL client certificate received by a
  client which is using SSL client authentication */

struct CertificateInfo
{
   std::string subject;
   std::string issuer;
   std::string notBefore;
   std::string notAfter;

   std::string serial;
   std::string cipher;
   std::string base64Value;
   std::string sha1;
};

//***************************************************************************
} // namespace cex

#endif // __SSL_HPP__

