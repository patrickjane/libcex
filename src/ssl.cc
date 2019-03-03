//*************************************************************************
// File ssl.cc
// Date 15.05.2018 - #1
// Copyright (c) 2018-2018 by Patrick Fial
//-------------------------------------------------------------------------
// Class Server / SSL functions
//*************************************************************************

//***************************************************************************
// includes
//***************************************************************************

#include <cex/core.hpp>

#ifdef CEX_WITH_SSL

#include <evhtp/sslutils.h>
#include <cex/ssl.hpp>
#include <time.h>

namespace cex
{

//***************************************************************************
// class Server
//***************************************************************************
// verify certificate
//***************************************************************************

int Server::verifyCert(int ok, X509_STORE_CTX * store) 
{
   char buf[256];

   X509* err_cert= X509_STORE_CTX_get_current_cert(store);
   int err= X509_STORE_CTX_get_error(store);
   int depth= X509_STORE_CTX_get_error_depth(store);

   SSL* ssl= (SSL*)X509_STORE_CTX_get_ex_data(store, SSL_get_ex_data_X509_STORE_CTX_idx());
   evhtp_connection_t* connection= (evhtp_connection_t*)SSL_get_app_data(ssl);
   evhtp_ssl_cfg_t* ssl_cfg= connection->htp->ssl_cfg;

   X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);

   if (ssl_cfg->verify_depth && depth > ssl_cfg->verify_depth) 
   {
      ok  = 0;
      err = X509_V_ERR_CERT_CHAIN_TOO_LONG;

      X509_STORE_CTX_set_error(store, err);
   }

//   if (!ok)
//      printf("SSL: verify error:num=%d:%s:depth=%d:%s\n", err, X509_verify_cert_error_string(err), depth, buf);

   return ok;
}

//***************************************************************************
// verify certificate
//***************************************************************************

void Server::setSslOption(const char* option, const char* value)
{
   if (!serverConfig.sslConfig)
      return;

   if (!strcmp(option, "cert"))
      serverConfig.sslConfig->pemfile= strdup(value);

   if (!strcmp(option, "key"))
      serverConfig.sslConfig->privfile= strdup(value);

   if (!strcmp(option, "ca"))
      serverConfig.sslConfig->cafile= strdup(value);

   if (!strcmp(option, "capath"))
      serverConfig.sslConfig->capath= strdup(value);

   if (!strcmp(option, "ciphers"))
      serverConfig.sslConfig->ciphers= strdup(value);

   if (!strcmp(option, "dhparams"))
      serverConfig.sslConfig->dhparams= strdup(value);

   if (!strcmp(option, "ecdh-name"))
      serverConfig.sslConfig->named_curve= strdup(value);

   if (!strcmp(option, "verify-depth"))
      serverConfig.sslConfig->verify_depth= atoi(value);

   if (!strcmp(option, "verify-client"))
      serverConfig.sslVerifyMode= htp_sslutil_verify2opts(value);

   if (!strcmp(option, "enable-cache"))
      serverConfig.sslConfig->scache_type= evhtp_ssl_scache_type_internal;

   if (!strcmp(option, "cache-timeout"))
      serverConfig.sslConfig->scache_timeout= atoi(value);

   if (!strcmp(option, "cache-size"))
      serverConfig.sslConfig->scache_size= atoi(value);

   if (!strcmp(option, "ctx-timeout"))
      serverConfig.sslConfig->ssl_ctx_timeout= atoi(value);

   if (!strcmp(option, "enable-protocol"))
   {
      if (!strcasecmp(value, "SSLv2"))
         serverConfig.sslConfig->ssl_opts &= ~SSL_OP_NO_SSLv2;

      else if (!strcasecmp(value, "SSLv3"))
         serverConfig.sslConfig->ssl_opts &= ~SSL_OP_NO_SSLv3;

      else if (!strcasecmp(value, "TLSv1"))
         serverConfig.sslConfig->ssl_opts &= ~SSL_OP_NO_TLSv1;

      else if (!strcasecmp(value, "ALL"))
         serverConfig.sslConfig->ssl_opts= 0;
   }

   if (!strcmp(option, "disable-protocol"))
   {
      if (!strcasecmp(value, "SSLv2"))
         serverConfig.sslConfig->ssl_opts |= SSL_OP_NO_SSLv2;

      else if (!strcasecmp(value, "SSLv3"))
         serverConfig.sslConfig->ssl_opts |= SSL_OP_NO_SSLv3;

      else if (!strcasecmp(value, "TLSv1"))
         serverConfig.sslConfig->ssl_opts |= SSL_OP_NO_TLSv1;

      else if (!strcasecmp(value, "ALL"))
         serverConfig.sslConfig->ssl_opts= SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1;
   }
}

//***************************************************************************
// get ssl client info (based on evhtp htp_sslutil_add_xheaders)
//***************************************************************************

void Server::getSslClientInfo(Request* req)
{
   evhtp_connection_t* connection= evhtp_request_get_connection(req->req);

   if (!connection || !connection->ssl || !htp_sslutil_cert_tostr(connection->ssl))
      return;

   CertificateInfo* cert= new CertificateInfo;

   cert->subject= (const char*)htp_sslutil_subject_tostr(connection->ssl);
   cert->issuer= (const char*)htp_sslutil_issuer_tostr(connection->ssl);
   cert->serial= (const char*)htp_sslutil_serial_tostr(connection->ssl);
   cert->cipher= (const char*)htp_sslutil_cipher_tostr(connection->ssl);
   cert->base64Value= (const char*)htp_sslutil_cert_tostr(connection->ssl);
   cert->sha1= (const char*)htp_sslutil_sha1_tostr(connection->ssl);
   cert->notBefore= (const char*)htp_sslutil_notbefore_tostr(connection->ssl);
   cert->notAfter= (const char*)htp_sslutil_notafter_tostr(connection->ssl);

   req->properties.set("sslClientCert", cert);
}

} // namespace cex

#endif // CEX_WITH_SSL

