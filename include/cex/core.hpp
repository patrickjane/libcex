//*************************************************************************
// File core.hpp
// Date 24.04.2018
// Copyright (c) 2018-2019 by Patrick Fial
//-------------------------------------------------------------------------
// cex Library core definitions
//*************************************************************************

#ifndef __CORE_HPP__
#define __CORE_HPP__

/*! \file core.hpp
  \brief `libcex` core classes and functions */
//***************************************************************************
// includes
//***************************************************************************

#include <evhtp/evhtp.h>
#include <event2/thread.h>

#include <thread>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <vector>
#include <regex>

#include <plist.hpp>
#include <cex/cex_config.h>

#define IO_BUFFER_SIZE 128*1024

namespace cex
{

//***************************************************************************
// definitions
//***************************************************************************

enum ReturnValues
{
   na=      -1,
   fail=    na,
   yes=     1,
   no=      0,
   success= 0,
   done=    0
};

/*!
  \public
  \brief Enumeration of available HTTP methods */
enum Method
{
   unknownMethod= na,

   methodGET,         // 0 /*!< HTTP GET method */
   methodHEAD,        // 1
   methodPOST,        // 2
   methodPUT,         // 3MaMa
   methodDELETE,      // 4
   methodOPTIONS,     // 5
   methodTRACE,       // 6
   methodCONNECT,     // 7
   methodPATCH,       // 8
   methodMKCOL,       // 9
   methodCOPY,        // 10
   methodMOVE,        // 11
   methodPROPFIND,    // 12
   methodPROPPATCH,   // 13
   methodLOCK,        // 14
   methodUNLOCK,      // 15

   nMethods
};

/*!
  \public
  \brief Enumeration of available HTTP versions (HTTP 1.0 or HTTP 1.1) */
enum Protocol
{
   unknownProtocol= na,

   protocol10,        // 0
   protocol11,        // 1

   nProtocols
};

class Request;
class Response;

/*! \brief Returns the library version as string */
const char* getLibraryVersion();

//***************************************************************************
// types
//***************************************************************************

typedef std::shared_ptr<Request> ReqPtr;
typedef std::shared_ptr<Response> ResPtr;

/*! \public
   \brief A function which is called by a standard Middleware when an incoming request matches.
   \param req The Request object representing the matched request 
   \param res The corresponding Response object which allows to create/send a response to the client 
   \param next A function to call when the next middleware shall be evaluated. */
typedef std::function<void(Request* req, Response* res, std::function<void()> next)> MiddlewareFunction;

/*! \public
   \brief A function which is called by an upload Middleware when an incoming request matches.

   The UploadFunction receives the incoming data stream as parameters, so the upload can be processed (e. g. stored 
   to disk or loaded into memory).

   The function is called repeatedly, depending on the size of the upload. Following middlewares will not be called
   until the upload is finished.
   \param req The Request object representing the matched request 
   \param data The current data block of the upload
   \param len The number of bytes of the current data block 
 */
typedef std::function<void(Request* req, const char* data, size_t len)> UploadFunction;

/*! \public
  \brief A callback function which receives a name and a value parameter
  \param name The name of the header/parameter
  \param value The value of the header/parameter
  \return Shall return `true` to abort iteration, and `false` to continue iteration */
typedef std::function<bool(const char* name, const char* value)> PairCallbackFunction;

typedef std::pair<std::string,bool> MimeType;
typedef std::unordered_map<std::string, MimeType> MimeTypes;
typedef std::unique_ptr<std::thread, std::function<void(std::thread* t)>> ThreadPtr;
typedef std::unique_ptr<event_base, std::function<void(event_base*)>> EventBasePtr;

//***************************************************************************
// class Request
//***************************************************************************
/*! \class Request
  \brief Contains the current request and all of its preparsed properties.

  Contains several accessor methods for retrieving header information, URL parameters or request bodies.

Example:
```
   app.use("/content", [](cex::Request* req, cex::Response* res, std::function<void()> next)
   {
      printf("Protocol: [%d], Method [%d], port [%d], host [%s], url [%s], path [%s], file [%s], user [%s], password [%s]\n",
            req->getProtocol(), 
            req->getMethod(), 
            req->getPort(), 
            req->getHost(), 
            req->getUrl(), 
            req->getPath(), 
            req->getFile(),
            req->properties.getString("basicUsername").c_str(), req->properties.getString("basicPassword").c_str());

      req->eachQueryParam([](const char* key, const char* value)
      {
         printf("PARAM: [%s] = [%s]\n", key, value);
         return true;
      });

      const char* body= req->getBody();

      res->end(200);
   });

```
*/

class Request
{
   friend class Server;
   friend class Response;
   friend class Middleware;

   public:

      /*! \brief Constructs a new Response object 
        \param req The underlying `libevhtp` request object 
       */
      explicit Request(evhtp_request* req);

     // base request info

      Method getMethod();      /*!< \brief Returns the request's HTTP method */
      Protocol getProtocol();  /*!< \brief Returns the HTTP protocol version (1.0 or 1.1) */
      int getPort();           /*!< \brief Returns the port, if present in the URI */
      const char* getHost();   /*!< \brief Returns the hostname of the request */
      const char* getUrl();    /*!< \brief Returns the full URL of the request */
      const char* getPath();   /*!< \brief Returns the path-portion of the URL of the request */
      const char* getFile();   /*!< \brief Returns the file-portion of the URL of the request */

      const char* getMiddlewarePath();   /*!< Returns the path of the currently matched Middleware */

      // HTTP header related

      /*! \brief Iterates all HTTP headers of the request with the given callback function
        \param cb A PairCallbackFunction which is called for each header.
        If the callback function returns `true`, iteration is stopped.*/
      void eachHeader(PairCallbackFunction cb); 

      /*! \brief Returns the value of a HTTP header 
        \param name Name of the HTTP header to retrieve */
      const char* get(const char* name);

      // URL query parameter related

      /*! \brief Iterates all URL query parameters of the request with the given callback function
        \param cb A PairCallbackFunction which is called for each parameter.
        If the callback function returns `true`, iteration is stopped.*/
      void eachQueryParam(PairCallbackFunction cb);
      
      /*! \brief Returns the value of a URL query parameter
        \param name Name of the parameter to retrieve */
      const char* getQueryParam(const char* name);

      // request body

      const char* getBody();     /*!< Returns the RAW body contents of the request  (unparsed, can be binary data)*/
      size_t getBodyLength();    /*!< Returns the length of RAW body contents (number of bytes) */
 
      // CEX properties (sessionId, sslClientCert, ...)

      /*! \brief A list of properties of the current request

        * Some properties are filled by `libcex`:
        \li `basicUsername` - The username, if the request was made with HTTP basic authentication
        \li `basicPassword` - The password, if the request was made with HTTP basic authentication
        \li `[sessionid]` - The session ID of the current request. Its created by the \link cex::sessionHandler \endlink middleware function,
        and the name of the property corresponds to the name of the cookie/sessionID as given in the middleware options */

      PropertyList properties;

   private:

      void parse();

      static int keyValueIteratorCb(evhtp_kv_t * kv, void * arg);

      evhtp_request* req;
      evhtp_path_t* uri;
      evhtp_authority_t* authority;

      int evhtp_method;
      int port;
      std::string host;
      Method method;
      Protocol protocol;
      std::string middlewarePath;
      std::vector<char> body;
};

//***************************************************************************
// class Response
//***************************************************************************
/*! \class Response
  \brief Serves as interface to the response which will be sent to the client. Contains functions for modifying HTTP headers and specifying response contents.

Example:
```
   app.use([](cex::Request* req, cex::Response* res, std::function<void()> next)
   {
      res->setFlags(res->getFlags() | cex::Response::fCompressGZip);

      res->set("Content-Type", "application/json");

      res->end("{\"id\": 10}", 200);
   });
 
```
  */

class Response
{
   public:

      /*! \brief State of the response. */
      enum State
      {
         stInit, /*!< Response is in pending state, and not yet completed. */
         stDone  /*!< Response was completed, that means a response was sent to the client. */
      };

      /*! \brief Flags describing features of the response. Currently this affects only compression. 
       
        For compression to work, the library must be compiled with zlib support.
       */
      enum Flags
      {
         fNoFlags=         0x0000,

         fCompression=     0x000F,
         fCompressGZip=    0x0001,  /*!< Enable GZip compression of the response contents */
         fCompressDeflate= 0x0002   /*!< Enable deflate compression of the response contents */
      };

      /*! \brief Constructs a new `Response` object 
        \param req The underlying `libevhtp` request object 
       */
      explicit Response(evhtp_request* req);

      /*! \brief Sets a HTTP header to a given value
        \param name Name of the HTTP header
        \param Value The value which shall be set
        */
      void set(const char* name, const char* value);
      
      /*! \brief Sets a HTTP header to a given value
        \param name Name of the HTTP header
        \param Value The value which shall be set
        */
      void set(const char* name, int value);

      /*! \brief Sends a response to the client with the supplied HTTP code and payload text 
       \param string The text which shall be sent to the client in the response body.
       \param status The HTTP code which shall be sent to the client.
       */ 
      int end(const char* string, int status);

      /*! \brief Sends a response to the client with the supplied HTTP code and payload buffer
       \param buffer The data buffer which holds the data which shall be sent to the client in the response body.
       \param bufLen The number of bytes which of the buffer which shall be sent.
       \param status The HTTP code which shall be sent to the client.
       */ 
      int end(const char* buffer, size_t bufLen, int status);
      
      /*! \brief Sends a response to the client with the supplied HTTP code and no body/payload
       \param status The HTTP code which shall be sent to the client.
       */ 
      int end(int status);

      /*! \brief Streams a response to the client with the supplied HTTP code
       \param status The HTTP code which shall be sent to the client.
       \param stream A pointer to a `std::istream` instance which is used to read the response contents from.
       \return `cex::success` (0) if the whole contents were successfully transferred or `cex::fail` (-1) if the stream could not be read.
      
       This function is useful for transferring larger payloads (e.g. files) which shall not be fully loaded into memory.
       */ 
      int stream(int status, std::istream* stream);

      /*! \brief Queries the state of the response.
        \param aState The state which shall be compared to the response object state
        \return `true` if the state of the object matches the supplied state, otherwise `false`.
       */
      bool isState(State aState) { return state == aState; }

      /*! \brief Checks if state of the response is `stDone`.
        \return `true` if the state of the object  is `stDone`, otherwise `false`.
       */
      bool isDone() { return isState(stDone); }

      /*! \brief Checks if state of the response is **not** `stDone`.
        \return `true` if the state of the object  is **not** `stDone`, otherwise `false`.
       */
      bool isPending() { return !isState(stDone); }

      /*! \brief Sets the response object flags 
        \param newFlags The flags which shall replace the currently set flags */
      void setFlags(int newFlags) { flags= newFlags; }

      /*! \brief Returns the currently set flags of the response object */
      int getFlags() { return flags; };

   private:

      static evhtp_res sendChunk(evhtp_connection_t* conn, void* arg);

      evhtp_request* req;
      State state;
      int flags;
};

//***************************************************************************
// class Middleware
//***************************************************************************
/*! \class Middleware 
  \brief Represents a single middleware.

  Contains the middleware function and its configuration (path, method, matching flags).
  */
class Middleware
{
   friend class Server;

   public:

      /*! \brief Flags for controlling the path matching behaviour */
      enum Flags
      {
         fMatchContain= 0x001,  /*!< Match if the request's URL contains the Middleware path */
         fMatchCompare= 0x002,  /*!< Match if the request's URL equals the Middleware path */
         fMatchRegex=   0x004,  /*!< Perform a regular expression match with the Middleware path as pattern */
         fMatching=     0x00F
      };

      /*! \brief Type of middleware */
      enum Type
      {
         tpStandard, /*!< Standard Middleware */
         tpUpload    /*!< Middleware to handle (file-)uploads */
      };

      /*! \brief Constructs a new middleware with the given parameters 
        \param path The path the URL shall be attached to. If path is NULL, all URLs will match 
        \param func The MiddlewareFunction which is called when the request matches 
        \param method The HTTP method the request must match 
        \param flags Matching flags which control the path matching behaviour */
      Middleware(const char* path, MiddlewareFunction func, int method= na, int flags= fMatchContain);
      Middleware(const char* path, UploadFunction func, int method= na, int flags= fMatchContain);

      bool match(Request* req);
      const char* getPath() { return path.c_str(); }

   private:

      int type;
      int method;
      int flags;
      std::regex rep;
      std::string path;
      MiddlewareFunction func;
      UploadFunction uploadFunc; 
};

//***************************************************************************
// class Server
//***************************************************************************
/*! \class Server
  \brief Core class of the embedded webserver. Manages a single HTTP/HTTPS listener
  and performs routing as defined by the installed middlewares.
*/
class Server
{
   public:

      /*! \struct Context
        \brief Internal helper struct for handling libevhtp callback functions
       */

      struct Context
      {
         Context(evhtp_request_t* request, Server* serv)
            : req(new Request(request)), res(new Response(request)), serv(serv) {}

         ReqPtr req;
         ResPtr res;
         Server* serv;
      };

      /*! \struct Config
        \brief Structure transporting all configuration options of the embedded
        server. Note that certain middlewares have additional config structs 
        (e. g. \ref cex::filesystem )
        */

      struct Config
      {
         Config();
         Config(Config& other);
         virtual ~Config();

         int port;              /*!< \brief HTTP/HTTPS listener port of the server. */
         std::string address;   /*!< \brief Bind address of the server */

         bool compress;         /*!< \brief Globally enable compression of outgoing responses (default: true).

                                  This will enable gzip/deflate compression of responses if Accept-Encoding allows compressioni (default: false).\n Compression can be enabled/disabled manually for a single request using the request flags. For example: `res.get()->setFlags(res.get()->getFlags() | Response::fCompressGZip)`. \n \n Library **must** be built with `libz` to make this work. */
         bool parseSslInfo;     /*!< \brief Flag indicating whether or not SSL client info shall be parsed for each request (default: true). 
                                  
                                  This tries to extract the SSL certificate provided by the client and store it into a CertificateInfo structure within the requests `sslClientCert` property. */
         bool sslEnabled;       /*!< \brief Flag indicating whether or not SSL is enabled on the listener (default: false). */
         int threadCount;       /*!< \brief Controls the number of worker threads the server is going to use (default: 4). */

#ifdef CEX_WITH_SSL
         int sslVerifyMode;
         evhtp_ssl_cfg_t* sslConfig;
#endif
      };

      /*! \brief Constructs a new server with the default config. */
      Server();

      /*! \brief Constructs a new server with the given config.
        \param config The Config object which defines the server options/configuration
       */
      explicit Server(Config& config);
      virtual ~Server();

      // server

      /*! \brief Starts the server with listener on address and port specified in the server Config struct 
        \param block If set to `true`, runs the listener/eventloop in the calling thread, thus blocking the caller.
        If set to `false`, spawns a new thread which runs the listener/eventloop, and returns immediately.*/
      int listen(bool block= true);

      /*! \brief Starts the server with listener on the given address and port 
        \param address The address to start the listener on (e.g. `localhost` or `10.0.2.14`)
        \param port The port to start the listener on
        \param block If set to `true`, runs the listener/eventloop in the calling thread, thus blocking the caller.
        If set to `false`, spawns a new thread which runs the listener/eventloop, and returns immediately.*/
      int listen(std::string address, int port, bool block= true);

      /*! \brief Stops the listener. If it was started within a background thread, the background thread is terminated. */
      int stop();

      // router

      /*! \brief Removes all attached middlewares */
      void reset() { middleWares.clear(); }

      /*! \brief Attaches a middleware function with no conditions 
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void use(MiddlewareFunction func);

      /*! \brief Attaches a middleware function with no HTTP method specification and with the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void use(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for HTTP GET requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void get(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the HTTP GET method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void get(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for HTTP PUT requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void put(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the HTTP PUT method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void put(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for HTTP POST requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void post(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the HTTP POST method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void post(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for HTTP HEAD requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void head(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the HTTP HEAD method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void head(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for HTTP DEL requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void del(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the HTTP DEL method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void del(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for HTTP CONNECT requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void connect(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the HTTP CONNECT method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void connect(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for HTTP OPTIONS requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void options(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the HTTP OPTIONS method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void options(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);


      /*! \brief Attaches a middleware function for HTTP TRACE requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void trace(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the HTTP TRACE method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void trace(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for HTTP PATCH requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void patch(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the HTTP PATCH method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void patch(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      // WEBDAV HTTP methods

      /*! \brief Attaches a middleware function for WEBDAV MKCOL requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void mkcol(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the WEBDAV MKCOL method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void mkcol(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for WEBDAV COPY requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void copy(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the WEBDAV COPY method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void copy(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for WEBDAV MOVE requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void move(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the WEBDAV MOVE method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void move(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for HTTP WEBDAV PROPFIND requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void propfind(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the WEBDAV PROPFIND method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void propfind(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for WEBDAV PROPPATCH requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void proppatch(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the WEBDAV PROPPATCH method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void proppatch(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for WEBDAV LOCK requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void lock(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the WEBDAV LOCK method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void lock(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      /*! \brief Attaches a middleware function for WEBDAV UNLOCK requests
        
        The function will be called for every request.
        \param func The middleware function which shall be called */
      void unlock(MiddlewareFunction func);

      /*! \brief Attaches a middleware function for the WEBDAV UNLOCK method and the given path

        The function will be called when the URL of incoming requests match the path, regardless of HTTP method.
        \param path The URL path which shall be compared against the request URL
        \param func The middleware function which shall be called
        \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void unlock(const char* path, MiddlewareFunction func, int flags= Middleware::fMatchContain);

      // upload hooks to catch file uploads w/ streaming

      /*! \brief Attaches a special middleware function which receives request body data uploads in chunks

         The provided UploadFunction will be called repeatedly until the whole request body data is received. Processing will not
         continue to any subsequent middlewares until the upload is completed.
         \param func The UploadFunction which shall be called upon receiving request body data */
      void uploads(UploadFunction func);

      /*! \brief Attaches an upload middleware function for the given HTTP method and URL path

         The provided UploadFunction will be called repeatedly for matching requests until the whole request body data is received. Processing will not
         continue to any subsequent middlewares until the upload is completed.
         \param path The URL path which shall be compared against the request URL
         \param func The UploadFunction which shall be called upon receiving request body data
         \param flags Flags controlling the URL matching behaviour (see Middleware)*/
      void uploads(const char* path, UploadFunction func, Method method= methodPOST, int flags= Middleware::fMatchContain);
 
      // tools

      static MimeTypes* getMimeTypes() { return mimeTypes.get(); }
      static void registerMimeType(const char* ext, const char* mime, bool binary);

      // SSL/TLS

#ifdef CEX_WITH_SSL
      void setSslOption(const char* option, const char* value);
      void getSslClientInfo(Request* req);
#endif

      static void libraryInit();

   private:

      int start(bool block);

      static int initMimeTypes();

      static void handleRequest(evhtp_request* req, void* arg);
      static evhtp_res handleHeaders(evhtp_request_t* request, evhtp_headers_t* hdr, void* arg);
      static evhtp_res handleBody(evhtp_request_t* req, struct evbuffer* buf, void* arg);
      static evhtp_res handleFinished(evhtp_request_t* req, void* arg);

#ifdef CEX_WITH_SSL
      static int verifyCert(int ok, X509_STORE_CTX* store);
#endif

      // members

      std::vector<std::unique_ptr<Middleware>> middleWares;
      std::vector<std::unique_ptr<Middleware>> uploadWares;

      Config serverConfig;

      // server control

      EventBasePtr eventBase;
      ThreadPtr backgroundThread;
      std::mutex startMutex;
      std::condition_variable startCond;
      bool startSignaled;
      bool started;

      // global/static stuff

      static bool initialized;
      static std::mutex initMutex;
      static std::unique_ptr<MimeTypes> mimeTypes;
};

//***************************************************************************
} // namespace cex

#endif // __CORE_HPP_
