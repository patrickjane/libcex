//*************************************************************************
// File util.hpp
// Date 24.04.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// Utility functions
//*************************************************************************

#ifndef __UTIL_HPP__
#define __UTIL_HPP__

/*! \file util.hpp 
  \brief Contains general utility and helper functions */
 
//***************************************************************************
// includes
//***************************************************************************

#include <string>
#include <vector>

struct evbuffer;

namespace cex
{

//***************************************************************************
// Definitions
//***************************************************************************

enum CompressionMode
{
   cmUnknown= -1,

   cmDeflate,
   cmGZip
};

//***************************************************************************
// Utility
//***************************************************************************

std::vector<std::string> splitString(const char* str, char delim = ',', int trim = 1);
std::string randomStringHex(int len);

#ifdef CEX_WITH_ZLIB
int compress(const char* src, size_t srcLen, struct evbuffer* dest, CompressionMode compMode= cmGZip);
int compress(std::istream* stream, std::function<void(char*,size_t)> onChunk, CompressionMode compMode);
#endif

static inline void lTrim(std::string &s) 
{
   s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) 
	    {
	       return !std::isspace(ch);
	    }));
}

static inline void rTrim(std::string &s) 
{
   s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) 
	    {
	       return !std::isspace(ch);
	    }).base(), s.end());
}

static inline void trim(std::string &s) 
{
   lTrim(s);
   rTrim(s);
}


static inline const char* notNull(const char* s) { return s ? s : ""; };
static inline int isEmpty(const char* s) { return !s || !strlen(s); };

//***************************************************************************
} // namespace cex

#endif // __UTIL_HPP_

