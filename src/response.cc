//*************************************************************************
// File request.cc
// Date 24.04.2018
// Copyright (c) 2018-2019 by Patrick Fial
//-------------------------------------------------------------------------
// cex Library core definitions
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <iostream>

#include <cex/core.hpp>
#include <cex/ssl.hpp>
#include <cex/util.hpp>

namespace cex
{
//***************************************************************************
// class Response
//***************************************************************************
// set (HTTP header)
//***************************************************************************

Response::Response(evhtp_request* req)
   : req(req), state(stInit) 
{
   flags= 0;
}

void Response::set(const char* headerName, const char* headerValue)
{
   if (!req || !req->headers_out)
      return;

   evhtp_header_key_add(req->headers_out, headerName, 1);
   evhtp_header_val_add(req->headers_out, headerValue, 1);
}

void Response::set(const char* headerName, int headerValue)
{
   if (!req || !req->headers_out)
      return;
   
   char number[100];
   sprintf(number, "%d", headerValue);

   evhtp_header_key_add(req->headers_out, headerName, 1);
   evhtp_header_val_add(req->headers_out, number, 1);
}

//***************************************************************************
// end (sent response payload)
//***************************************************************************

int Response::end(const char* string, int status)
{
   return end(string, strlen(string)+1, status);
}

int Response::end(const char* buf, size_t bufLen, int status)
{
   if (state == stDone)
      return done;

   if (!buf || bufLen <= 0)
      return fail;

   auto* buffer= req->buffer_out;

   if (!buffer)
      return fail;

#ifdef CEX_WITH_ZLIB
   if (flags & fCompression)
   {
      compress((char*)buf, bufLen, buffer, flags & fCompressGZip ? cmGZip : cmDeflate);
      set("Content-Encoding", flags & fCompressGZip ? "gzip" : "deflate");
   }
   else
#endif
   {
      evbuffer_add(buffer, buf, bufLen);
   }

   evhtp_send_reply_start(req, status);
   evhtp_send_reply_body(req, buffer);
   evhtp_send_reply_end(req);

   state= stDone;
   return done;
}

int Response::end(int status)
{
   if (state == stDone)
      return done;

   evhtp_send_reply(req, status);
   state= stDone;
   return done;
}

//***************************************************************************
// stream (sent response payload w/ streaming)
//***************************************************************************

int Response::stream(int status, std::istream* stream)
{
   size_t bytesRead= -1;
   char ioBuffer[IO_BUFFER_SIZE];
   evbuffer* sendBuffer;

   if (!stream || !stream->good())
   {
      evhtp_send_reply(req, status);
      return fail;
   }

   sendBuffer= evbuffer_new();

   // compression, if enabled

#ifdef CEX_WITH_ZLIB
   if ((flags & fCompression))
   {
      evhtp_request* thisReq= req;

      auto onChunk = [&sendBuffer, &thisReq](char* buf, size_t bufLen)
      { 
         evbuffer_add(sendBuffer, buf, bufLen);
         evhtp_send_reply_chunk(thisReq, sendBuffer);
         evbuffer_drain(sendBuffer, bufLen);
      };

      set("Content-Encoding", flags & fCompressGZip ? "gzip" : "deflate");

      evhtp_send_reply_chunk_start(req, EVHTP_RES_OK);
      compress(stream, onChunk, (flags & fCompressGZip) ? cmGZip : cmDeflate);
   }
   else
#endif
   {
      evhtp_send_reply_chunk_start(req, EVHTP_RES_OK);

      while (!stream->eof() && stream->good())
      {
         stream->read(ioBuffer, IO_BUFFER_SIZE);
         bytesRead= stream->gcount();

         if (bytesRead <= 0)
            break;

         evbuffer_add(sendBuffer, ioBuffer, bytesRead);
         evhtp_send_reply_chunk(req, sendBuffer);
         evbuffer_drain(sendBuffer, bytesRead);
      }
   }

   evhtp_send_reply_chunk_end(req);
   evhtp_safe_free(sendBuffer, evbuffer_free);

   return done;
}

//***************************************************************************
} // namespace cex

