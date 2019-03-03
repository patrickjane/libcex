//*************************************************************************
// File security.cc
// Date 07.05.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// Security headers middleware
// Set several HTTP headers according to security options
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <cex/security.hpp>

namespace cex
{

//***************************************************************************
// Middleware securityHeaders
//***************************************************************************

static struct SecurityOptions defaultOptions;

SecurityOptions::SecurityOptions() 
{ 
   ieNoOpen= disableCache= noSniff= xssProtection= true; 
   noDNSPrefetch= yes; 
   
   referrer= refUnknown;
   xFrameAllow= xfUnknown; 

   stsMaxAge= 31536000; 
   stsIncludeSubDomains= false; 
   stsPreload= false;

   hpkpMaxAge= 31536000; 
   hpkpIncludeSubDomains= true;
}

MiddlewareFunction securityHeaders(std::shared_ptr<SecurityOptions> opts)
{
   // opts is CAPTURED, thus held for the lifetime of the lambda. this is INTENDED, and NOT a leak,
   // so the shared_ptr is not an issue

   MiddlewareFunction res = [opts](Request* req, Response* res, std::function<void()> next)
   {
      SecurityOptions* theOpts = opts.get() ? opts.get() : &defaultOptions;

      // X-DNS-Prefetch-Control

      if (theOpts->noDNSPrefetch != na)
         res->set("X-DNS-Prefetch-Control", theOpts->noDNSPrefetch ? "off" : "on");

      // X-Frame-Options

      if (theOpts->xFrameAllow != xfUnknown)
      {
         if (theOpts->xFrameAllow == xfFrom)
         {
            std::string from("ALLOW-FROM " + theOpts->xFrameFrom);
            res->set("X-Frame-Options", from.c_str());
         }
         else
         {
            res->set("X-Frame-Options", theOpts->xFrameAllow == xfDeny ? "DENY" : "SAMEORIGIN");
         }
      }

      // Public-Key-Pins

      if (theOpts->hpkpMaxAge > 0 && theOpts->hpkpKeys.size())
      {
         std::string pin;
         std::vector<std::string>::iterator it= theOpts->hpkpKeys.begin();
         char age[100];

         sprintf(age, "%d", theOpts->hpkpMaxAge);

         while (it != theOpts->hpkpKeys.end())
         {
            if (it != theOpts->hpkpKeys.begin())
               pin += "; ";

            pin += "pin-sha256=\"";
            pin += *it;
            pin += "\"";

            it++;
         }

         pin += "; max-age=";
         pin += age;

         if (theOpts->hpkpIncludeSubDomains)
            pin += "; includeSubdomains";
            
         if (theOpts->hpkpReportUri.length())
         {
            pin += "; report-uri=\"";
            pin += theOpts->hpkpReportUri;
            pin += "\"";
         }

         res->set("Public-Key-Pins", pin.c_str());
      }

      // Strict-Transport-Security

      if (theOpts->stsMaxAge > 0)
      {
         std::string sts("max-age=");
         char age[100];

         sprintf(age, "%d", theOpts->stsMaxAge);
         sts += age;

         if (theOpts->stsIncludeSubDomains)
            sts += "; includeSubdomains";
         
         if (theOpts->stsPreload)
            sts += "; preload";

         res->set("Strict-Transport-Security", sts.c_str());
      }

      // X-Download-Options

      if (theOpts->ieNoOpen)
         res->set("X-Download-Options", "noopen");

      // some anti-caching headers

      if (theOpts->disableCache)
      {
         res->set("Cache-Control", "no-store, no-cache, must-revalidate, proxy-revalidate");
         res->set("Pragma", "no-cache");
         res->set("Expires", "0");
      }

      // X-Content-Type-Options

      if (theOpts->noSniff)
         res->set("X-Content-Type-Options", "nosniff");

      // Referrer-Policy

      if (theOpts->referrer != na)
      {
         switch (theOpts->referrer)
         {
            case refNoReferrer:                  res->set("Referrer-Policy", "no-referrer"); break;
            case refNoReferrerWhenDowngrade:     res->set("Referrer-Policy", "no-referrer-when-downgrade"); break;
            case refSameOrigin:                  res->set("Referrer-Policy", "same-origin"); break;
            case refOrigin:                      res->set("Referrer-Policy", "origin"); break;
            case refStrictOrigin:                res->set("Referrer-Policy", "strict-origin"); break;
            case refOriginWhenCrossOrigin:       res->set("Referrer-Policy", "origin-when-cross-origin"); break;
            case refStrictOriginWhenCrossOrigin: res->set("Referrer-Policy", "strict-origin-when-cross-origin"); break;
            case refUnsafeUrl:                   res->set("Referrer-Policy", "unsafe-url"); break;
            default:
               break;
         }
      }

      // X-XSS-Protection

      if (theOpts->xssProtection)
         res->set("X-XSS-Protection", "1; mode=block");

      next();
   };

   return res;
}

//***************************************************************************
} // namespace cex

