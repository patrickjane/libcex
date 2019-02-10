# libcex
## Overview
A C++11 embedded webserver framework.

Focuses on the concept of middleware functions to provide an extremely easy to use and easy to setup API.

The most basic example might be as simple as:

```
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
```
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

# Usage
## Server
The `cex::Server` class provides the HTTP/HTTPS listener and actually processes the request received by clients.
A server can be created with default options (see API docs) or concrete options:

```
cex::Server app;

// or

cex::Server::Config cfg;
cex::Server app(&cfg);
```

The listener is started once the `listen` method is called:

```
app.listen(true);
```

Supplying `true` to the last parameter starts the listener/eventloop within the calling thread. If `false` is provided, a background thread will be spawned for the listener/eventloop, and the call to `listen` returns immediately.   


**Note**: The background thread only servers for the eventloop. The actual request processing might use additional/more threads as given by the `threadCount` config option (default: 4).

## Middlewares
### Basics
The `cex::Server` class also provides the interface to attach middleware functions.
Each middleware function will receive the `cex::Request` and `cex::Response` objects, which allow to interact with the currently receiced request as well as construct responses which will be sent back to the client.

Middleware functions can be attached for a certain HTTP method, a certain URL, or globally (without restrictions).

Example:

```
// global middleware 

app.use([](cex::Request* req, cex::Response* res, std::function<void()> next) { ... });

// middleware only for HTTP GET 

app.get([](cex::Request* req, cex::Response* res, std::function<void()> next) { ... });

// middleware only for HTTP GET and path /content

app.get("/content", app.use([](cex::Request* req, cex::Response* res, std::function<void()> next) { ... });

// middleware for any HTTP method and path /content

app.use("/content", app.use([](cex::Request* req, cex::Response* res, std::function<void()> next) { ... });

```

The actual function supplied to `cex::Server::use` (and variants) can be a function pointer or a lambda.

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

# Copyright notice
`libcex` uses the following two awesome libraries for unit tests:

- [bandit](https://github.com/banditcpp/bandit) - Human-friendly unit testing for C++11
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - A C++11 header-only HTTP library
