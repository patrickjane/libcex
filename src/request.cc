//*************************************************************************
// File request.cc
// Date 24.04.2018 - #1
// Copyright (c) 2018-2019 by Patrick Fial
//-------------------------------------------------------------------------
// cex Library Request class implementation
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
// class Request
//***************************************************************************
// ctor/dtor
//***************************************************************************

Request::Request(evhtp_request* req) 
   : req(req)
{
   parse();
}

//***************************************************************************
// get (header value)
//***************************************************************************

const char* Request::get(const char* headerName) 
{ 
   if (!req || !req->headers_in)
      return 0; 

   return evhtp_header_find(req->headers_in, headerName);
}

//***************************************************************************
// eachHeader
//***************************************************************************

void Request::eachHeader(PairCallbackFunction cb)
{
   if (!req || !req->headers_in)
      return;

   evhtp_headers_for_each(req->headers_in, &Request::keyValueIteratorCb, &cb);
}

//***************************************************************************
// get query param
//***************************************************************************

const char* Request::getQueryParam(const char* name) 
{ 
   if (!req || !req->uri || !req->uri->query)
      return 0;

   evhtp_kv_t* keyValue= evhtp_kvs_find_kv((evhtp_kvs_t*)req->uri->query, name);

   return keyValue ? keyValue->val : 0;
}

//***************************************************************************
// each query param
//***************************************************************************

void Request::eachQueryParam(PairCallbackFunction cb)
{
   if (!req || !req->uri || !req->uri->query)
      return;

   evhtp_kvs_for_each((evhtp_kvs_t*)req->uri->query, &Request::keyValueIteratorCb, &cb);
}

//***************************************************************************
// keyValueIteratorCb
//***************************************************************************

int Request::keyValueIteratorCb(evhtp_kv_t * kv, void * arg)
{
   bool stop= (*((PairCallbackFunction*)arg))(kv ? (const char*)kv->key : 0, kv ? (const char*)kv->val : 0);

   if (stop)
      return 1;

   return 0;
}

//***************************************************************************
// get*
//***************************************************************************

const char* Request::getBody()
{
   return body.data();
}

size_t Request::getBodyLength()
{
   return body.size();
}

int Request::getPort()
{
   return port;
}

Method Request::getMethod() 
{ 
   return method;
}

Protocol Request::getProtocol() 
{ 
   return protocol;
}

const char* Request::getHost() 
{ 
   return host.c_str();
}

const char* Request::getUrl() 
{ 
   return notNull(uri ? uri->full : "");
}

const char* Request::getPath() 
{ 
   return notNull(uri ? uri->path : "");
}

const char* Request::getFile() 
{ 
   return notNull(uri ? uri->file : "");
}

const char* Request::getMiddlewarePath() 
{ 
   return middlewarePath.c_str();
}

//***************************************************************************
// parse
//***************************************************************************

void Request::parse()
{
   // need some additional parsing on top of libevhtp

   port= na;
   method= unknownMethod;

   uri= req->uri && req->uri->path ? req->uri->path : 0;
   authority= req->uri ? req->uri->authority : 0;
   evhtp_method= req->method;

   switch (evhtp_method)
   {
      case htp_method_GET:       method= methodGET;       break;
      case htp_method_HEAD:      method= methodHEAD;      break;
      case htp_method_POST:      method= methodPOST;      break;
      case htp_method_PUT:       method= methodPUT;       break;
      case htp_method_DELETE:    method= methodDELETE;    break;
      case htp_method_OPTIONS:   method= methodOPTIONS;   break;
      case htp_method_TRACE:     method= methodTRACE;     break;
      case htp_method_CONNECT:   method= methodCONNECT;   break;
      case htp_method_PATCH:     method= methodPATCH;     break;
      case htp_method_MKCOL:     method= methodMKCOL;     break;
      case htp_method_COPY:      method= methodCOPY;      break;
      case htp_method_MOVE:      method= methodMOVE;      break;
      case htp_method_PROPFIND:  method= methodPROPFIND;  break;
      case htp_method_PROPPATCH: method= methodPROPPATCH; break;
      case htp_method_LOCK:      method= methodLOCK;      break;
      case htp_method_UNLOCK:    method= methodUNLOCK;    break;
      default:
         break;
   }

   if (req->proto == EVHTP_PROTO_10)
      protocol= protocol10;
   else if (req->proto == EVHTP_PROTO_11)
      protocol= protocol11;
   else
      protocol= unknownProtocol;

   // parse host

   const char* hostHeader= get("Host");

   if (hostHeader)
   {
      const char* p= strchr(hostHeader, ':');

      if (p)
      {
         port= atoi(p+1);
         host.assign(hostHeader, p-hostHeader);
      }
      else
         host.assign(hostHeader);
   }
}

//***************************************************************************
} // namespace cex

