/*!
 * \file conn_pool.h
 * \author Nathan Eloe
 * \brief A ZMQ connection pool
 * From: https://github.com/neloe/ngx_zmq
 * BSD-2-Clause license
 */

#ifndef CONN_POOL_H
#define CONN_POOL_H
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <zmq.h>

typedef struct _conn
{
  struct _conn *m_next;
  void *m_sock;
} conn;

typedef struct
{
  char *m_endpt;
  void *m_ctx;
  int m_stype;
  int m_to;
  conn *m_front, *m_back;
} connpool;

connpool* init_pool( void* ctx, ngx_pool_t* mpool, int stype );
void set_endpt( connpool* cp, ngx_str_t endpt );
void set_socktype( connpool * cp, const int stype);
conn* init_conn(connpool* cp);
conn* get_conn(connpool* cp);
void  rel_conn(connpool* cp, conn** con);
void free_conn(conn** con);
void free_pool(connpool** cp);

#endif