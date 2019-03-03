//*************************************************************************
// File server.cc
// Date 24.04.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
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
// class Server
//***************************************************************************
// statics
//***************************************************************************

bool Server::initialized= false;
std::mutex Server::initMutex;
std::unique_ptr<MimeTypes> Server::mimeTypes(new MimeTypes);

const char* getLibraryVersion()
{
   return CEX_VERSION;
}

//***************************************************************************
// ctor/dtor
//***************************************************************************

Server::Server(Config& config)
   : serverConfig(config)
{
   libraryInit();

   startSignaled= started= false;
}

Server::Server() 
{ 
   libraryInit();
   startSignaled= started= false;
}

Server::~Server() 
{
}

//***************************************************************************
// init
//***************************************************************************

void Server::libraryInit()
{
   initMutex.lock();

   if (!initialized)
   {
      // must be called ONCE before all other libevent calls (!)
      // otherwise threading/locking will fail/cause issues.

      evthread_use_pthreads();

      // mime type map, init once.

      initMimeTypes();
      initialized= true;
   }

   initMutex.unlock();
}

//***************************************************************************
// listen
//***************************************************************************

int Server::listen(bool block)
{
   if (!serverConfig.address.length() || serverConfig.port == na)
      return fail;

   return start(block);
}

int Server::listen(std::string aAddress, int aPort, bool block) 
{ 
   serverConfig.address= aAddress;
   serverConfig.port= aPort;

   return listen(block);
}

//***************************************************************************
// start
//***************************************************************************

int Server::start(bool block)
{
   // create eventloop & http server
   // in non-blocking mode, start in separate background thread
   // this is NOT the thread for the request handlers. request handlers will
   // have separate worker threads ANYWAY as supplied by the 'threadCount' parameter
   // && set by evhtp_use_threads() function

   if (started)
      throw std::runtime_error("Server already started");

   std::runtime_error err("");

   auto startFunc= [this, &block, &err]()
   {
      if (!block)
         startMutex.lock();

      eventBase= EventBasePtr(event_base_new(), &event_base_free);
      std::unique_ptr<evhtp_t, decltype(&evhtp_free)> httpServer(evhtp_new(eventBase ? eventBase.get() : 0, NULL), &evhtp_free);

      if (!eventBase)
      {
         err= std::runtime_error("Failed to create new base_event.");
         return;
      }

      if (!httpServer)
         throw std::runtime_error("Failed to create new evhtp.");

#ifdef CEX_WITH_SSL
      if (serverConfig.sslEnabled)
      {
         if (serverConfig.sslVerifyMode) 
         {
            serverConfig.sslConfig->verify_peer= serverConfig.sslVerifyMode;
            serverConfig.sslConfig->x509_verify_cb = Server::verifyCert;
         }

         evhtp_ssl_init(httpServer.get(), serverConfig.sslConfig);
      }
#endif

      // attach static request callback function, init number of threads, bind socket
      // DONT use evhtp_set_gencb, because we need the return-cb to attach the
      // evhtp_hook_on_headers callback function HERE.

      //evhtp_set_gencb(httpServer.get(), Server::handleRequest, this);

      auto cb= evhtp_set_cb(httpServer.get(), "", Server::handleRequest, this);

      evhtp_callback_set_hook(cb, evhtp_hook_on_headers, (evhtp_hook)Server::handleHeaders, this);
      evhtp_bind_socket(httpServer.get(), serverConfig.address.c_str(), serverConfig.port, 128);

      // function 'evhtp_use_threads' is marked deprecated, but according to libevhtp source
      // the function which should be used now (evhtp_use_threads_wexit) will be renamed to evhtp_use_threads at some point o_O
      
      if (serverConfig.threadCount > 1 && initialized)
         evhtp_use_threads_wexit(httpServer.get(), NULL, NULL, serverConfig.threadCount, NULL);
//         evhtp_use_threads(httpServer.get(), NULL, serverConfig.threadCount, NULL);

      started= true;

      if (!block)
      {
         startSignaled= true;
         startCond.notify_one();
         startMutex.unlock();
      }

      // this BLOCKS the current thread

      event_base_loop(eventBase.get(), 0);

      // when done, properly unbind httpSever. will be free'd by unique_ptr
      // event_base will be free'd by stop()

      evhtp_unbind_socket(httpServer.get());
   };

   // execute the main start function from main/calling thread ...
   // (forced if library was not setup for threading)

   if (block || !Server::initialized)
   {
      startFunc();
      return success;
   }

   // ... OR from background thread

   std::unique_lock<std::mutex> lock(startMutex);

   backgroundThread= ThreadPtr(new std::thread(startFunc), [this](std::thread *t) 
   { 
      // safe to call from different thread context as long as we have used
      // evthread_use_pthreads() (which is the case in Server::init())

      event_base_loopexit(eventBase.get(), NULL);
      t->join(); 
      delete t; 
   });

   // wait until thread actually up & running (wait cond unlocks the mutex)

   while (!startSignaled)
      startCond.wait(lock);

   return success;
}

//***************************************************************************
// stop
//***************************************************************************

int Server::stop()
{
   if (!eventBase || !started)
      return done;

   // delete (stop) thread or just break out of the event loop

   if (backgroundThread)
      backgroundThread.reset();
   else
      event_base_loopexit(eventBase.get(), NULL); 

   started= startSignaled= false;

   return done;
}

//***************************************************************************
// use (general middleware)
//***************************************************************************

void Server::use(MiddlewareFunction func)
{
   use(0, func);
}

//***************************************************************************
// use (routing middleware)
//***************************************************************************

void Server::use(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, na, flags))));
}

//***************************************************************************
// use (method variants)
//***************************************************************************

void Server::get(MiddlewareFunction func)
{
   get(0, func);
}

void Server::get(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_GET, flags))));
}

void Server::put(MiddlewareFunction func)
{
   put(0, func);
}

void Server::put(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_PUT, flags))));
}

void Server::post(MiddlewareFunction func)
{
   post(0, func);
}

void Server::post(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_POST, flags))));
}

void Server::head(MiddlewareFunction func)
{
   head(0, func);
}

void Server::head(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_HEAD, flags))));
}

void Server::del(MiddlewareFunction func)
{
   del(0, func);
}

void Server::del(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_DELETE, flags))));
}

void Server::connect(MiddlewareFunction func)
{
   connect(0, func);
}

void Server::connect(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_CONNECT, flags))));
}

void Server::options(MiddlewareFunction func)
{
   options(0, func);
}

void Server::options(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_OPTIONS, flags))));
}

void Server::trace(MiddlewareFunction func)
{
   trace(0, func);
}

void Server::trace(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_TRACE, flags))));
}

void Server::patch(MiddlewareFunction func)
{
   patch(0, func);
}

void Server::patch(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_PATCH, flags))));
}


void Server::mkcol(MiddlewareFunction func)
{
   mkcol(0, func);
}

void Server::mkcol(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_MKCOL, flags))));
}

void Server::copy(MiddlewareFunction func)
{
   copy(0, func);
}

void Server::copy(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_COPY, flags))));
}

void Server::move(MiddlewareFunction func)
{
   move(0, func);
}

void Server::move(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_MOVE, flags))));
}

void Server::propfind(MiddlewareFunction func)
{
   propfind(0, func);
}

void Server::propfind(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_PROPFIND, flags))));
}

void Server::proppatch(MiddlewareFunction func)
{
   proppatch(0, func);
}

void Server::proppatch(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_PROPPATCH, flags))));
}

void Server::lock(MiddlewareFunction func)
{
   lock(0, func);
}

void Server::lock(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_LOCK, flags))));
}

void Server::unlock(MiddlewareFunction func)
{
   unlock(0, func);
}

void Server::unlock(const char* path, MiddlewareFunction func, int flags)
{
   middleWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, htp_method_UNLOCK, flags))));
}

// upload hooks to catch file uploads w/ streaming

void Server::uploads(UploadFunction func)
{
   uploads(0, func);
}

void Server::uploads(const char* path, UploadFunction func, Method method, int flags)
{
   int m= htp_method_POST;

   switch (method)
   {
      case methodGET:       m= htp_method_GET; break;
      case methodHEAD:      m= htp_method_HEAD; break;
      case methodPOST:      m= htp_method_POST; break;
      case methodPUT:       m= htp_method_PUT; break;
      case methodDELETE:    m= htp_method_DELETE; break;
      case methodOPTIONS:   m= htp_method_OPTIONS; break;
      case methodTRACE:     m= htp_method_TRACE; break;
      case methodCONNECT:   m= htp_method_CONNECT; break;
      case methodPATCH:     m= htp_method_PATCH; break;
      case methodMKCOL:     m= htp_method_MKCOL; break;
      case methodCOPY:      m= htp_method_COPY; break;
      case methodMOVE:      m= htp_method_MOVE; break;
      case methodPROPFIND:  m= htp_method_PROPFIND; break;
      case methodPROPPATCH: m= htp_method_PROPPATCH; break;
      case methodLOCK:      m= htp_method_LOCK; break;
      case methodUNLOCK:    m= htp_method_UNLOCK; break;
      default:
         break;
   }

   uploadWares.push_back(std::move(std::unique_ptr<Middleware>(new Middleware(path, func, m, flags))));
}
 
//***************************************************************************
// handle headers (step 1)
//***************************************************************************

evhtp_res Server::handleHeaders(evhtp_request_t* request, evhtp_headers_t* hdr, void* arg)
{
   // initially create a context object which will be used throughout the workflow.
   // holds the request, response and server pointers.

   Server* serv= (Server*)arg;
   Server::Context* ctx= new Server::Context(request, serv);

   // add hooks for body upload & finish of request. 'handleRequest' was already registered
   // in Server::listen

   evhtp_request_set_hook(request, evhtp_hook_on_read, (evhtp_hook)Server::handleBody, ctx); 
   evhtp_request_set_hook(request, evhtp_hook_on_request_fini, (evhtp_hook)Server::handleFinished, ctx); 

   return EVHTP_RES_OK;
}

//***************************************************************************
// handle upload (step 2)
//***************************************************************************

evhtp_res Server::handleBody(evhtp_request_t* req, struct evbuffer* buf, void* arg)
{
   Server::Context* ctx= (Server::Context*)arg;
   std::vector<char>* body= &(ctx->req.get()->body);

   if (!body)                 // should never happen
      return EVHTP_RES_OK;

   size_t bytesReady= evbuffer_get_length(buf);
   size_t oldSize= body->size();

   // (1) check if we have attached upload middleware(s)

   if (ctx->serv->uploadWares.size())
   {
      std::vector<std::unique_ptr<Middleware>>::iterator it= ctx->serv->uploadWares.begin();

      while (it != ctx->serv->uploadWares.end())
      {
         if (!((*it).get()->match(ctx->req.get())))
         {
            it++;
            continue;
         }

         // we found a matching upload ware. copy bytes into body buffer (CURRENT CHUNK ONLY)
         // and call the upload middleware function.

         body->resize(bytesReady);

         if (body->size() < bytesReady)
         {
            // allocation error

            return EVHTP_RES_500;
         }

         ev_ssize_t bytesCopied= evbuffer_copyout(buf, (void*)(body->data()), bytesReady);


         ctx->req.get()->middlewarePath= (*it).get()->getPath();
         (*it).get()->uploadFunc(ctx->req.get(), body->data(), bytesCopied);

         return EVHTP_RES_OK;
      }
   }

   // (2) no upload middleware attached, or none matched. just copy bytes into (full) body buffer

   body->resize(bytesReady + oldSize);

   if (body->size() < (bytesReady + oldSize))
   {
      // allocation error

      return EVHTP_RES_500;
   }

   evbuffer_copyout(buf, (void*)(body->data() + oldSize), bytesReady);

   // drain, so libevhtp won't copy it into the native request's internal buffer (req->buffer_in)

   evbuffer_drain(buf, evbuffer_get_length(buf));

   return EVHTP_RES_OK;
}
 
//***************************************************************************
// handle upload (step 3)
//***************************************************************************

void Server::handleRequest(evhtp_request* req, void* arg)
{
   // actual request handling & middlewares as soon as we have headers + body ready.
   // the context is stored within the req as the cb-argument for the evhtp_hook_on_request_fini-hook.

   Server* serv= (Server*)arg;
   Server::Context* ctx= req && req->hooks ? (Server::Context*)req->hooks->on_request_fini_arg : nullptr;

   if (!ctx)
   {
      // should NEVER happen

      evhtp_send_reply(req, 500);
      return;
   }

   // retrieve SSL client info (certificate), if available & configured

#ifdef CEX_WITH_SSL
   if (ctx->serv->serverConfig.parseSslInfo)
      ctx->serv->getSslClientInfo(ctx->req.get());
#endif

   // enable compression, if available & configured

#ifdef CEX_WITH_ZLIB
   if (ctx->serv->serverConfig.compress)
   {
      const char* acceptEncoding= ctx->req.get()->get("Accept-Encoding");

      if (acceptEncoding && strstr(acceptEncoding, "gzip"))
         ctx->res.get()->setFlags(ctx->res.get()->getFlags() | Response::fCompressGZip);
      else if (acceptEncoding && strstr(acceptEncoding, "deflate"))
         ctx->res.get()->setFlags(ctx->res.get()->getFlags() | Response::fCompressDeflate);
   }
#endif

   // call all registered handlers (route-based and general middlewares)

   std::vector<std::unique_ptr<Middleware>>::iterator it= ctx->serv->middleWares.begin();

   if (ctx->serv->middleWares.size())
      it = ctx->serv->middleWares.begin();

   if (!ctx->serv->middleWares.size() || it == ctx->serv->middleWares.end())
   {
      ctx->res.get()->end(404);
      return;
   }

   std::function<void()> next;

   // create next-function to be handed to each middleware

   next = [&next, &ctx, &it]()
   {
      it++;

      if (it != ctx->serv->middleWares.end())
      {
         if ((*it).get()->match(ctx->req.get()))
         {
            ctx->req.get()->middlewarePath= (*it).get()->getPath();
            (*it).get()->func(ctx->req.get(), ctx->res.get(), next);
         }
         else
            next();
      }
   };

   // call next handler OR next-function, if next handler doesnt match.
   // if no middleware matched, the request will hang (thats intended).

   if ((*it).get()->match(ctx->req.get()))
   {
      ctx->req.get()->middlewarePath= (*it).get()->getPath();
      (*it).get()->func(ctx->req.get(), ctx->res.get(), next);
   }
   else
      next();
}

//***************************************************************************
// handle finished (step 4)
//***************************************************************************

evhtp_res Server::handleFinished(evhtp_request_t* req, void* arg)
{
   // forget the request context we created

   Server::Context* ctx= (Server::Context*)arg;

   delete ctx;
   
   return EVHTP_RES_OK;
}

//***************************************************************************
// class Server::Config
//***************************************************************************
// ctor
//***************************************************************************

Server::Config::Config() 
{ 
   port= na;
   compress= true; 
   parseSslInfo= true; 
   sslEnabled= false;
   threadCount= 4; 

#ifdef CEX_WITH_SSL
   sslVerifyMode= 0;
   sslConfig= (evhtp_ssl_cfg_t*)calloc(1, sizeof(evhtp_ssl_cfg_t));
   sslConfig->ssl_opts = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1;
   sslConfig->scache_type= evhtp_ssl_scache_type_disabled;
   sslConfig->scache_size= 0;
   sslConfig->scache_timeout= 0;
#endif
}

Server::Config::Config(Config& other)
{
   port= na;
   compress= other.compress;
   parseSslInfo= other.parseSslInfo;
   sslEnabled= other.sslEnabled;
   threadCount= other.threadCount;

#ifdef CEX_WITH_SSL
   sslVerifyMode= other.sslVerifyMode;
   sslConfig= (evhtp_ssl_cfg_t*)calloc(1, sizeof(evhtp_ssl_cfg_t));
   memcpy(sslConfig, other.sslConfig, sizeof(evhtp_ssl_cfg_t));
#endif
}

Server::Config::~Config()
{
#ifdef CEX_WITH_SSL
   ::free(sslConfig->pemfile);
   ::free(sslConfig->privfile);
   ::free(sslConfig->cafile);
   ::free(sslConfig->capath);
   ::free(sslConfig->ciphers);
   ::free(sslConfig->dhparams);
   ::free(sslConfig->named_curve);
   ::free(sslConfig->ciphers);     
   ::free(sslConfig);
#endif
}

//***************************************************************************
} // namespace cex

