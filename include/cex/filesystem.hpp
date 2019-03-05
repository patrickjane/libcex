//*************************************************************************
// File filesystem.hpp
// Date 24.05.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// Filesystem functions
// Middleware that loads content from local filesystem
//*************************************************************************

#ifndef __FILESYSTEM_HPP__
#define __FILESYSTEM_HPP__

/*! \file filesystem.hpp 
  \brief Filesystem middleware function

  Loads file contents from the local filesystem with the paths provided by the
  request URLs.
 
Example:
 ```
   std::shared_ptr<cex::FilesystemOptions> opts(new cex::FilesystemOptions());

   opts.get()->rootPath= "/my/root/path;

   app.use("/docs", cex::filesystem(opts));
 ```
 The `Content-Type` header is set accordingly for the mimetype of the queried file. The mimetype is determined by
 the file extension in the request-URL. `libcex` contains a list of known mimetypes to look up the exact `Content-Type` as 
 well as binary/text information.

 The `defaultEncoding` is added to the `Content-Type` if it was set and the determined mimetype is not a binary type.

 If no mimetype could be found in the internal list, `Content-Type` falls back to `text/plain` with the `defaultEncoding`.
 
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
// Security
//***************************************************************************

/*! \struct FilesystemOptions
  \brief Contains all options for the filesystem middleware
  */
 
struct FilesystemOptions
{
   /*! \brief Constructs a new options object with defaultEncoding `utf-8` and empty rootPath*/
   FilesystemOptions() : defaultEncoding("utf-8") {}

   std::string rootPath;         /*!< \brief Specifies the root-path on the local filesystem

                                  The path of request URLs will be appended as relative paths when accessing files. */
   std::string defaultEncoding;  /*!< \brief The default encoding set in the `Content-Type` header */
};

/*! \public 
  \brief Creates a middleware that loads files from the filesystem with the given path as rootPath
  \param path The path to use as rootPath

  The path of request URLs will be appended as relative paths when accessing files .
 */
MiddlewareFunction filesystem(std::string aPath);

/*! \public 
  \brief Creates a middleware that loads files from the filesystem with the given options object
  \param opts The FilesystemOptions object containing rootPath and defaultEncoding

  The path of request URLs will be appended as relative paths when accessing files .
 */
MiddlewareFunction filesystem(std::shared_ptr<FilesystemOptions> opts= nullptr);

//***************************************************************************
} // namespace cex

#endif // __FILESYSTEM_HPP_
