[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

# libcex
## Overview
A C++11 embedded webserver framework.

Focuses on the concept of middleware functions to provide an extremely easy to use API.

The most basic example might be as simple as:

```cpp
#include <cex.hpp>

int main()
{
   cex::Server app;
   
   app.use([](cex::Request* req, cex::Response* res, std::function<void()> next)
   {
      res->end(200);
   });
   
   app.listen("127.0.0.1", 5555, true);

   return 0;   
}

```
## Dependencies
`libcex`  requires the following libraries:

- [libevhtp](https://github.com/criticalstack/libevhtp)
- OpenSSL (optional) - for HTTPS support
- zlib (optional) - for compression of response payloads

# Installation
`libcex` uses the `cmake` build system to compile the library and testcases. To compile/install, simply do:

```
$ git clone https://github.com/patrickjane/libcex .
remote: Enumerating objects: 4, done.
remote: Counting objects: 100% (4/4), done.
remote: Compressing objects: 100% (3/3), done.
remote: Total 4 (delta 0), reused 0 (delta 0), pack-reused 0
Unpacking objects: 100% (4/4), done.

$ mkdir build
$ cd build
$ cmake ..
$ make
```

If cmake cannot find your OpenSSL installation, or you've installed in a non-standard location, you might want to add `-DOPENSSL_ROOT_DIR=/path/to/ssl` to the cmake call.


### Testcases
After successfully compiling the library, testcases can be run with `ctest`:

```
$ ctest
Test project /Users/patrickjane/Development/libcex/build
    Start 1: filesystem
1/5 Test #1: filesystem .......................   Passed    0.10 sec
    Start 2: mw_security
2/5 Test #2: mw_security ......................   Passed    0.04 sec
    Start 3: mw_session
3/5 Test #3: mw_session .......................   Passed    0.04 sec
    Start 4: routing
4/5 Test #4: routing ..........................   Passed    0.15 sec
    Start 5: uploads
5/5 Test #5: uploads ..........................   Passed    0.06 sec

100% tests passed, 0 tests failed out of 5

Total Test time (real) =   0.41 sec
```
# API
For a full API documentation, visit the doxygen site at: https://patrickjane.github.io/libcex/index.html

# Usage
## Server
[cex::Server API docs ↗](https://patrickjane.github.io/libcex/classcex_1_1_server.html)    

The `cex::Server` class provides the HTTP/HTTPS listener and actually processes the request received by clients.
A server can be created with default options (see API docs) or concrete options:

```cpp
cex::Server app;

// or

cex::Server::Config cfg;
cex::Server app(&cfg);
```

The listener is started once the `listen` method is called:

```cpp
app.listen(true);
```

Supplying `true` to the last parameter starts the listener/eventloop within the calling thread. If `false` is provided, a background thread will be spawned for the listener/eventloop, and the call to `listen` returns immediately.   


**Note**: The background thread only servers for the eventloop. The actual request processing might use additional/more threads as given by the `threadCount` config option (default: 4).

## Middlewares
[cex::Middleware API docs ↗](https://patrickjane.github.io/libcex/classcex_1_1_middleware.html)    

The `cex::Server` class also provides the interface to attach middleware functions.
Each middleware function will receive the `cex::Request` and `cex::Response` objects, which allow to interact with the currently receiced request as well as construct responses which will be sent back to the client.

Middleware functions can be attached for a certain HTTP method, a certain URL, or globally (without restrictions).

Example:

```cpp
// global middleware matching all incoming requests

app.use([](cex::Request* req, cex::Response* res, std::function<void()> next) { ... });

// middleware only for HTTP GET requests

app.get([](cex::Request* req, cex::Response* res, std::function<void()> next) { ... });

// middleware only for HTTP GET and path /content

app.get("/content", app.use([](cex::Request* req, cex::Response* res, std::function<void()> next) { ... });

// middleware for any HTTP method and path /content

app.use("/content", app.use([](cex::Request* req, cex::Response* res, std::function<void()> next) { ... });

```

Middleware functions can be a function pointer, function object or a lambda.

--- 

**!! Attention !!**    
Since, depending on the thread count, requests can be processes by a random thread, attached middleware functions must be **reentrant**.

However, requests will only be processed within one thread, no matter how many middlewares are attached.

--- 
### Processing logic
For each incoming request, all attached middlewares are evaluated. If a request matches the middleware's HTTP method and URL, the middleware function is executed. The middlewares are executed in the order they were registered.

Each middleware function receives the following three parameters:

- The `cex::Request` object contaning everything about the incoming request
- The `cex::Response` object which is used to create a response
- A function pointer which shall be used/called to skip to the next middleware

Execution of middlewares stops once:

- the last registered middleware was executed
- the `next` method of a middleware was not called

### Built-in middlewares
`libcex` already provides a few predefined middleware functions ready to use:

- `cex::filesystem` middleware for accesing static files on the filesystem [(API docs ↗)](https://patrickjane.github.io/libcex/filesystem_8hpp.html)
- `cex::security` middleware that sets a number of security related HTTP headers [(API docs ↗)](https://patrickjane.github.io/libcex/security_8hpp.html)
- `cex::basicAuth` middleware that extracts HTTP basic auth information from the request [(API docs ↗)](https://patrickjane.github.io/libcex/basicauth_8hpp.html)
- `cex::sessionHandler` middleware that adds/retrieves session cookies from/to requests/responses [(API docs ↗)](https://patrickjane.github.io/libcex/session_8hpp.html)

## Requests
[cex::Request API docs ↗](https://patrickjane.github.io/libcex/classcex_1_1_request.html)    

The `cex::Request` class provides access to the request contents (URL, headers, parameters, body, ...) as sent by the client. An instance of `cex::Request` represents a single HTTP request which shall be handled by the application. In terms of HTTP communication, `cex::Request` is *read only*, that is, it cannot be used to send a response. For this, `cex::Response` is used.

Example:

```cpp
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
     req->properties.getString("basicUsername").c_str(), 
     req->properties.getString("basicPassword").c_str());

     req->eachQueryParam([](const char* key, const char* value)
     {
        printf("PARAM: [%s] = [%s]\n", key, value);
        return true;
     });

     const char* body= req->getBody();
     
     // do something with body ...
});    

```
### Properies
To allow middlewares to transfer information between them, the `cex::Request` class contains a property list. For example, the `cex::basicAuth` middleware stored the username and password supplied by the client in the properties `basicUsername` and `basicPassword`.

## Response
[cex::Response API docs ↗](https://patrickjane.github.io/libcex/classcex_1_1_response.html)    

The `cex::Response` class provides the interface for sending responses back to the client. This includes the HTTP Code, payloads as well as header parameters. 
The most simple response might just include the HTTP code:

```cpp
app.use("/content", [](cex::Request* req, cex::Response* res, std::function<void()> next)
{
   res->end(200); // HTTP 200 OK
});
```

The response class also allows to set headers:

```cpp
   res->set("Content-Type", "text/plain");
```

... or send a payload:

```cpp
   res->end("Hello world :)", 200)
```

## Advanced topics
### File uploads
`libcex` allows incoming file uploads using a special form of middleware function, the `cex::UploadFunction`. It is different from the usual middlewares in that it is called repeatedly for a single request, each time providing a chunk of upload data. In addition, upload functions are executed **before** middlewares, that is, the first middleware function is only called **after** all of the upload has been received.

Note that `libcex` does not buffer incoming data. It is up to the application to handle uploaded data within the `cex::UploadFunction`. 

The `cex::Server` class provides an interface to attach upload functions:

```cpp
app.uploads("/uploads", [&uploadBuffer](cex::Request* req, const char* data, size_t len)
{
   int fd= -1;

   if (!req->properties.has("uploadFileHandle"))
   {
      fd= ::open("myfile", O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
      req->properties.set("uploadFileHandle", (long)fd);
   }
   else
   {
      fd= req->properties.getLong("uploadFileHandle");
   }

   ::write(fd, data, len);
});
```

don't forget to close the file descriptor:

```cpp
app.post([](cex::Request* req, cex::Response* res, std::function<void()> next)
{
   if (req->properties.has("uploadFileHandle"))
   {
      int fd= req->properties.getLong("uploadFileHandle");
      ::close(fd);

      req->properties.remove("uploadFileHandle");
   }

   res->end(200);
});
```

### Sending large responses
In case a response shall contain a large payload, using `cex::Response::end` would lead to the entire response beeing kept in memory, which might be undesirable.     
To solve this issue, `libcex` provides a streaming API for sending responses: 

```cpp
app.get("/myfile", [](cex::Request* req, cex::Response* res, std::function<void()> next)
{
   std::ifstream file;
   file.open("myfile", std::ios_base::in|std::ios_base::binary);

   res->set("Content-Type", "application/octet-stream");
   res->stream(200, &file);
});
```
The `cex::Response::stream` function accepts a `std::istream`, such as a `std::ifstream`.

# Copyright notice
`libcex` uses the following two awesome libraries for unit tests:

- [bandit](https://github.com/banditcpp/bandit) - Human-friendly unit testing for C++11
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - A C++11 header-only HTTP library
