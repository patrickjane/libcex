//*************************************************************************
// File mime.cc
// Date 24.05.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// cex MIME type definitions
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <cex/core.hpp>

namespace cex
{

//***************************************************************************
// initMimetypes
//***************************************************************************

int Server::initMimeTypes()
{
   if (mimeTypes.get()->size())
      return done;

   registerMimeType("ai", "application/postscript", false);
   registerMimeType("aif", "audio/x-aiff", true);
   registerMimeType("aifc", "audio/x-aiff", true);
   registerMimeType("aiff", "audio/x-aiff", true);
   registerMimeType("asd", "application/astound", false);
   registerMimeType("asn", "application/astound", false);
   registerMimeType("au", "audio/basic", true);
   registerMimeType("avi", "video/x-msvideo", true);
   registerMimeType("bcpio", "application/x-bcpio", false);
   registerMimeType("bin", "application/octet-stream", true);
   registerMimeType("cab", "application/x-shockwave-flash", false);
   registerMimeType("cdf", "application/x-netcdf", false);
   registerMimeType("chm", "application/mshelp", false);
   registerMimeType("cht", "audio/x-dspeeh", true);
   registerMimeType("class", "application/octet-stream", true);
   registerMimeType("cod", "image/cis-cod", true);
   registerMimeType("com", "application/octet-stream", true);
   registerMimeType("cpio", "application/x-cpio", false);
   registerMimeType("csh", "application/x-csh", false);
   registerMimeType("css", "text/css", false);
   registerMimeType("csv", "text/comma-separated-values", false);
   registerMimeType("dcr", "application/x-director", false);
   registerMimeType("dir", "application/x-director", false);
   registerMimeType("dll", "application/octet-stream", true);
   registerMimeType("doc", "application/msword", false);
   registerMimeType("docx", "document", false);
   registerMimeType("dot", "application/msword", false);
   registerMimeType("dus", "audio/x-dspeeh", true);
   registerMimeType("dvi", "application/x-dvi", false);
   registerMimeType("dwf", "drawing/x-dwf", false);
   registerMimeType("dwg", "application/acad", false);
   registerMimeType("dxf", "application/dxf", false);
   registerMimeType("dxr", "application/x-director", false);
   registerMimeType("eps", "application/postscript", false);
   registerMimeType("es", "audio/echospeech", true);
   registerMimeType("etx", "text/x-setext", false);
   registerMimeType("evy", "application/x-envoy", false);
   registerMimeType("exe", "application/octet-stream", true);
   registerMimeType("fh4", "image/x-freehand", true);
   registerMimeType("fh5", "image/x-freehand", true);
   registerMimeType("fhc", "image/x-freehand", true);
   registerMimeType("fif", "image/fif", true);
   registerMimeType("gif", "image/gif", true);
   registerMimeType("gtar", "application/x-gtar", false);
   registerMimeType("gz", "application/gzip", true);
   registerMimeType("hdf", "application/x-hdf", false);
   registerMimeType("hlp", "application/mshelp", false);
   registerMimeType("hqx", "application/mac-binhex40", true);
   registerMimeType("htm", "text/html", false);
   registerMimeType("html", "text/html", false);
   registerMimeType("ico", "image/x-icon", true);
   registerMimeType("ief", "image/ief", true);
   registerMimeType("jpe", "image/jpeg", true);
   registerMimeType("jpeg", "image/jpeg", true);
   registerMimeType("jpg", "image/jpeg", true);
   registerMimeType("js", "text/javascript", false);
   registerMimeType("json", "application/json", false);
   registerMimeType("latex", "application/x-latex", false);
   registerMimeType("man", "application/x-troff-man", false);
   registerMimeType("mbd", "application/mbedlet", false);
   registerMimeType("mcf", "image/vasa", true);
   registerMimeType("me", "application/x-troff-me", false);
   registerMimeType("mid", "audio/x-midi", true);
   registerMimeType("midi", "audio/x-midi", true);
   registerMimeType("mif", "application/mif", false);
   registerMimeType("mov", "video/quicktime", true);
   registerMimeType("movie", "video/x-sgi-movie", true);
   registerMimeType("mp2", "audio/x-mpeg", true);
   registerMimeType("mpe", "video/mpeg", true);
   registerMimeType("mpeg", "video/mpeg", true);
   registerMimeType("mpg", "video/mpeg", true);
   registerMimeType("nc", "application/x-netcdf", false);
   registerMimeType("nsc", "application/x-nschat", false);
   registerMimeType("oda", "application/oda", false);
   registerMimeType("pbm", "image/x-portable-bitmap", true);
   registerMimeType("pdf", "application/pdf", true);
   registerMimeType("pgm", "image/x-portable-graymap", true);
   registerMimeType("php", "application/x-httpd-php", false);
   registerMimeType("phtml", "application/x-httpd-php", false);
   registerMimeType("png", "image/png", true);
   registerMimeType("pnm", "image/x-portable-anymap", true);
   registerMimeType("pot", "application/mspowerpoint", false);
   registerMimeType("ppm", "image/x-portable-pixmap", true);
   registerMimeType("pps", "application/mspowerpoint", false);
   registerMimeType("ppt", "application/mspowerpoint", false);
   registerMimeType("ppz", "application/mspowerpoint", false);
   registerMimeType("ps", "application/postscript", false);
   registerMimeType("ptlk", "application/listenup", false);
   registerMimeType("qt", "video/quicktime", true);
   registerMimeType("ra", "audio/x-pn-realaudio", true);
   registerMimeType("ram", "audio/x-pn-realaudio", true);
   registerMimeType("ras", "image/cmu-raster", true);
   registerMimeType("rgb", "image/x-rgb", true);
   registerMimeType("roff", "application/x-troff", false);
   registerMimeType("rpm", "audio/x-pn-realaudio-plugin", true);
   registerMimeType("rtc", "application/rtc", false);
   registerMimeType("rtf", "text/rtf", false);
   registerMimeType("rtx", "text/richtext", false);
   registerMimeType("sca", "application/x-supercard", false);
   registerMimeType("sgm", "text/x-sgml", false);
   registerMimeType("sgml", "text/x-sgml", false);
   registerMimeType("sh", "application/x-sh", false);
   registerMimeType("shar", "application/x-shar", false);
   registerMimeType("shtml", "text/html", false);
   registerMimeType("sit", "application/x-stuffit", false);
   registerMimeType("smp", "application/studiom", false);
   registerMimeType("spc", "text/x-speech", false);
   registerMimeType("spl", "application/futuresplash", false);
   registerMimeType("sprite", "application/x-sprite", false);
   registerMimeType("src", "application/x-wais-source", false);
   registerMimeType("stream", "audio/x-qt-stream", true);
   registerMimeType("sv4cpio", "application/x-sv4cpio", false);
   registerMimeType("sv4crc", "application/x-sv4crc", false);
   registerMimeType("swf", "application/x-shockwave-flash", false);
   registerMimeType("svg", "image/svg+xml", false);
   registerMimeType("t", "application/x-troff", false);
   registerMimeType("talk", "text/x-speech", false);
   registerMimeType("tar", "application/x-tar", true);
   registerMimeType("tgz", "application/gzip", true);
   registerMimeType("tbk", "application/toolbook", false);
   registerMimeType("tcl", "application/x-tcl", false);
   registerMimeType("tex", "application/x-tex", false);
   registerMimeType("texi", "application/x-texinfo", false);
   registerMimeType("texinfo", "application/x-texinfo", false);
   registerMimeType("tif", "image/tiff", true);
   registerMimeType("tiff", "image/tiff", true);
   registerMimeType("ttf", "application/x-font-ttf", true);
   registerMimeType("tr", "application/x-troff", false);
   registerMimeType("troff", "application/x-troff-me", false);
   registerMimeType("tsi", "audio/tsplayer", true);
   registerMimeType("tsp", "application/dspname", false);
   registerMimeType("tsv", "text/tab-separated-values", false);
   registerMimeType("txt", "text/plain", false);
   registerMimeType("ustar", "application/x-ustar", false);
   registerMimeType("viv", "vivo", false);
   registerMimeType("vivo", "vivo", false);
   registerMimeType("vmd", "application/vocaltec-media-desc", false);
   registerMimeType("vmf", "application/vocaltec-media-file", false);
   registerMimeType("vox", "audio/voxware", true);
   registerMimeType("wav", "audio/x-wav", true);
   registerMimeType("wbmp", "wbmp", false);
   registerMimeType("wml", "wml", false);
   registerMimeType("wmlc", "wmlc", false);
   registerMimeType("wmls", "wmlscript", false);
   registerMimeType("wmlsc", "wmlscriptc", false);
   registerMimeType("woff", "font/woff", true);
   registerMimeType("woff2", "font/woff2", true);
   registerMimeType("wrl", "model/vrml", false);
   registerMimeType("xbm", "image/x-xbitmap", true);
   registerMimeType("xhtml", "application/xhtml+xml", false);
   registerMimeType("xla", "application/msexcel", false);
   registerMimeType("xls", "application/msexcel", false);
   registerMimeType("xlsx", "sheet", false);
   registerMimeType("xml", "text/xml", false);
   registerMimeType("xpm", "image/x-xpixmap", true);
   registerMimeType("xwd", "image/x-windowdump", true);
   registerMimeType("z", "application/x-compress", false);
   registerMimeType("zip", "application/zip", true);

   return done;
}

//***************************************************************************
// register mime types 
//***************************************************************************

void Server::registerMimeType(const char* extension, const char* mime, bool binary)
{
   (*mimeTypes.get())[std::string(extension)]= std::make_pair(mime, binary);
}

//***************************************************************************
} // namespace cex

