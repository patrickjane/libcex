//*************************************************************************
// File security.hpp
// Date 07.05.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// Security functions
// Middleware that sets a crapload of security related HTTP headers
//*************************************************************************

#ifndef __SECURITY_HPP__
#define __SECURITY_HPP__

/*! \file security.hpp 
  \brief Security middleware function

  Sets a number of HTTP headers which serve the purpose of security.
 
Example:
 ```
   // using default headers

   app.use(cex::securityHeaders());
 ```*/


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
// Security
//***************************************************************************

/*! \enum XFrame
  \brief X-Frame header options / variants */
enum XFrame
{
   xfUnknown= na,

   xfDeny,
   xfSameOrigin,
   xfFrom
};

/*! \enum Referer
  \brief Referer header options / variants */
enum Referer
{
   refUnknown= na,

   refNoReferrer,
   refNoReferrerWhenDowngrade,
   refSameOrigin,
   refOrigin,
   refStrictOrigin,
   refOriginWhenCrossOrigin,
   refStrictOriginWhenCrossOrigin,
   refUnsafeUrl
};

/*! \struct SecurityOptions
  \brief Contains all options for the sessionHandler middleware
 
Example:
```
   std::shared_ptr<cex::SecurityOptions> opts(new cex::SecurityOptions());

   opts.get()->hpkpKeys.push_back("someKey");
   opts.get()->hpkpKeys.push_back("someKey2");
   opts.get()->hpkpKeys.push_back("someKey3");
   opts.get()->hpkpMaxAge= 183400;
   opts.get()->xFrameAllow= cex::xfFrom;
   opts.get()->xFrameFrom= "my.domain.de";
   opts.get()->stsMaxAge= 183400;
   opts.get()->stsPreload= true;
   opts.get()->ieNoOpen= cex::no;
   opts.get()->noDNSPrefetch= cex::no;
   opts.get()->referrer= cex::refOriginWhenCrossOrigin;

   app.use(cex::securityHeaders(opts));
```
will set the following headers:

```
X-DNS-Prefetch-Control: on
X-Frame-Options: ALLOW-FROM my.domain.de
Public-Key-Pins: pin-sha256="someKey"; pin-sha256="someKey2"; pin-sha256="someKey3"; max-age=183400; includeSubdomains
Strict-Transport-Security: max-age=183400; preload
Cache-Control: no-store, no-cache, must-revalidate, proxy-revalidate
Pragma: no-cache
Expires: 0
X-Content-Type-Options: nosniff
Referrer-Policy: origin-when-cross-origin
X-XSS-Protection: 1; mode=block
```
 */
struct SecurityOptions
{
   SecurityOptions();

   int noDNSPrefetch;  /*!< \brief If `> -1`, sets the `X-DNS-Prefetch-Control` header to `on` (1) or `off` (0) (default: `1`) */
   bool ieNoOpen;      /*!< \brief If `true`, sets the `X-Download-Options` header to `noopen` (default: `true`) */ 
   bool disableCache;  /*!< \brief If `true`, sets several cache-related headers (default: `true`)

                        Sets the following headers:
                        \li `Cache-Control: no-store, no-cache, must-revalidate, proxy-revalidate`
                        \li `Pragma: no-cache`
                        \li `Expires: 0` */
   bool noSniff;       /*!< \brief If `true`, sets the `X-Content-Type-Options` header to `nosniff` (default: `true`) */ 
   int referrer;       /*!< \brief If `<> refUnknown`, sets the Referrer-Policy header (default: `refUnknown`)

                        Sets the header according to the supplied enum value:

                        \li refNoReferrer sets `Referrer-Policy: no-referrer`
                        \li refNoReferrerWhenDowngrade sets `Referrer-Policy: no-referrer-when-downgrade`
                        \li refSameOrigin sets `Referrer-Policy: same-origin`
                        \li refOrigin sets `Referrer-Policy: origin`
                        \li refStrictOrigin sets `Referrer-Policy: strict-origin`
                        \li refOriginWhenCrossOrigin sets `Referrer-Policy: origin-when-cross-origin`
                        \li refStrictOriginWhenCrossOrigin sets `Referrer-Policy: strict-origin-when-cross-origin`
                        \li refUnsafeUrl sets `Referrer-Policy: unsafe-url`
                       */
 
   bool xssProtection; /*!< \brief If `true`, sets the `X-XSS-Protection` header to `1; mode=block` (default: `true`) */ 


   // X-Frame-Options

   XFrame xFrameAllow; /*!< \brief If <> `xfUnknown`, sets the `X-Frame-Options` header (default: `xfUnknown`) 
                        
                        Sets the header according to the supplied value:

                        \li `xfFrom` sets `X-Frame-Options: ALLOW-FROM ` plus the supplied string in `xFrameFrom` option
                        \li `xfDeny` sets `X-Frame-Options: DENY`
                        \li `xfSameOrigin` sets `X-Frame-Options: SAMEORIGIN`
                        */

   std::string xFrameFrom; /*!< \brief Sets the ALLOW-FROM name if `xFrameAllow` option is set to `xfFrom` */

   // Strict Transport Security

   int stsMaxAge;             /*!< \brief If `> 0`, sets the `Strict-Transport-Security` header (default: `31536000`) */
   bool stsIncludeSubDomains; /*!< \brief If the STS header is set, adds the `includeSubdomains` option (default: `false`) */
   bool stsPreload;           /*!< \brief If the STS header is set, adds the `preload` option (default: `false`) */

   // HTTP Public Key Pinning

   int hpkpMaxAge;                     /*!< \brief Sets the `max-age` option of the `Public-Key-Pins` header (default: `31536000`). 
                                         If `hpkpMaxAge` is not set, or `hpkpKeys` is empty, NO header is set*/

   std::vector<std::string> hpkpKeys;  /*!< \brief Should contain the list of keys to add to the HPKP header .
                                         If `hpkpMaxAge` is not set, or `hpkpKeys` is empty, NO header is set*/
   bool hpkpIncludeSubDomains;         /*!< \brief If the HPKP header is set, adds the `includeSubdomains` option (default: `true`) */
   std::string hpkpReportUri;          /*!< \brief If the HPKP header is set, adds the `report-uri` option, if the string is non-empty (default: empty) */
};

/*! \public 
  \brief Creates a middleware that sets a number of HTTP headers related to security
 */
MiddlewareFunction securityHeaders(std::shared_ptr<SecurityOptions> opts= nullptr);

//***************************************************************************
} // namespace cex

#endif // __SECURITY_HPP_
