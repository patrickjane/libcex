//*************************************************************************
// File util.cc
// Date 24.04.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// Utility functions
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <stdio.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <algorithm>

#ifdef CEX_WITH_SSL
#  include <openssl/err.h>
#  include <openssl/rand.h>
#endif

#ifdef CEX_WITH_ZLIB
#  include <zlib.h>
#endif

#include <cex/util.hpp>
#include <cex/core.hpp>

namespace cex
{

//***************************************************************************
// Split string 
//***************************************************************************

std::vector<std::string> splitString(const char* str, char delim, int doTrim)
{
   std::stringstream ss(str);
   std::vector<std::string> result;

   while (ss.good())
   {
      std::string substr;
      getline(ss, substr, delim);

      if (doTrim)
	 trim(substr);

      result.push_back(std::move(substr));
   }

   return result;
}

//***************************************************************************
// Random string (in hex) 
//***************************************************************************

std::string randomStringHex(int len)
{
   std::string res;

   if (len <= 0 || len >= 1024*1024)
      return res;

   unsigned char* buffer = (unsigned char*)calloc(len, sizeof(char));
   char buf[10];

#ifdef CEX_WITH_SSL
   int rc = RAND_bytes(buffer, len);

   if (rc != 1)
   {
      ::free(buffer);
      return res;
   }

   for (int i= 0; i < len; i++)
   {
      sprintf(buf, "%02X", buffer[i]);
      res.append(buf);
   }
#else
   std::srand(std::time(0));

   for (int i = 0; i < len; i++)
   {
      sprintf(buf, "%02X", (unsigned char)std::rand());
      res.append(buf);
   }

#endif

   ::free(buffer);

   return res;
}

#ifdef CEX_WITH_ZLIB
//***************************************************************************
// compress buffer (GZIP or deflate)
//***************************************************************************

int compress(const char* src, size_t srcLen, struct evbuffer* dest, CompressionMode compMode)
{
   if (!src || !dest)
      return fail;

   int res, flush;
   size_t bytesRead= 0;
   char out[IO_BUFFER_SIZE];

   int windowBits = 15;
   int GZIP_ENCODING = 16;
   z_stream strm;

   strm.zalloc = Z_NULL;
   strm.zfree = Z_NULL;
   strm.opaque = Z_NULL;

   if (compMode == cmGZip)
      res = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);
   else
      res = deflateInit(&strm, Z_DEFAULT_COMPRESSION);

   if (res != Z_OK)
      return res;

   // compress until end of input

   do 
   {
      size_t nextChunkLen = srcLen-bytesRead < IO_BUFFER_SIZE ? srcLen-bytesRead : IO_BUFFER_SIZE;

      flush = nextChunkLen < IO_BUFFER_SIZE ? Z_FINISH : Z_NO_FLUSH;
      strm.avail_in = nextChunkLen;
      strm.next_in = (Bytef*)(src+bytesRead);

      bytesRead += nextChunkLen;

      // run deflate() on input until output buffer not full, finish
      // compression if all of src has been read in

      do 
      {
         size_t bytesCompressed= 0;
         strm.avail_out = IO_BUFFER_SIZE;
         strm.next_out = (Bytef*)out;

         res = deflate(&strm, flush);

         if (res == Z_STREAM_ERROR)
            return res;

         bytesCompressed= IO_BUFFER_SIZE - strm.avail_out;

         evbuffer_add(dest, out, bytesCompressed);
      } 
      while (strm.avail_out == 0);
   } 
   while (flush != Z_FINISH);

   deflateEnd(&strm);

   return done;
}

//***************************************************************************
// compress stream (GZIP or deflate)
//***************************************************************************

int compress(std::istream* stream, std::function<void(char*,size_t)> onChunk, CompressionMode compMode)
{
   if (!stream || !onChunk || !stream->good() || stream->eof())
      return fail;

   int res, flush;
   size_t bytesRead= 0;
   char in[IO_BUFFER_SIZE];
   char out[IO_BUFFER_SIZE];

   int windowBits = 15;
   int GZIP_ENCODING = 16;
   z_stream strm;

   strm.zalloc = Z_NULL;
   strm.zfree = Z_NULL;
   strm.opaque = Z_NULL;

   if (compMode == cmGZip)
      res = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);
   else
      res = deflateInit(&strm, Z_DEFAULT_COMPRESSION);

   if (res != Z_OK)
      return res;

   // compress until end of input

   do 
   {
      stream->read(in, IO_BUFFER_SIZE);
      size_t nextChunkLen = stream->gcount();

      flush = nextChunkLen < IO_BUFFER_SIZE ? Z_FINISH : Z_NO_FLUSH;
      strm.avail_in = nextChunkLen;
      strm.next_in = (Bytef*)in;

      bytesRead += nextChunkLen;

      // run deflate() on input until output buffer not full, finish
      // compression if all of src has been read in

      do 
      {
         size_t bytesCompressed= 0;
         strm.avail_out = IO_BUFFER_SIZE;
         strm.next_out = (Bytef*)out;

         res = deflate(&strm, flush);

         if (res == Z_STREAM_ERROR)
            return res;

         bytesCompressed= IO_BUFFER_SIZE - strm.avail_out;
         onChunk(out, bytesCompressed);
      } 
      while (strm.avail_out == 0);
   } 
   while (flush != Z_FINISH);

   deflateEnd(&strm);

   return done;
}

#endif // CEX_WITH_ZLIB

//***************************************************************************
} // namespace cex

