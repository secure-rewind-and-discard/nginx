/**
 * @file ngx_http_parse_wrap.c
 * @author Merve Gulmez 
 * @brief 
 * @version 0.1
 * @date 2022-02-07
 * 
 * @copyright © Ericsson AB 2022-2023
 * 
 * SPDX-License-Identifier: BSD 3-Clause
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "../../../../secure-rewind-and-discard/src/sdrad_api.h"
#include "ngx_http_parse_wrap.h"


int start_routine = 0; 
int data_domain_flag = 0;

ngx_http_request_t  *r_copy;
ngx_buf_t           *header_copy;
ngx_pool_t          *pool_copy; 

void *
__ngx_memalign(int udi, size_t alignment, size_t size, ngx_log_t *log)
{
    void  *p;
    int    err;

    err = sdrad_memalign(udi, &p, alignment, size);

    if (err) {
        ngx_log_error(NGX_LOG_EMERG, log, err,
                      "posix_memalign(%uz, %uz) failed", alignment, size);
        p = NULL;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_ALLOC, log, 0,
                   "posix_memalign: %p:%uz @%uz", p, size, alignment);

    return p;
}


ngx_pool_t *
__ngx_create_pool(int udi, size_t size, ngx_log_t *log)
{
    ngx_pool_t  *p;

    p = __ngx_memalign(udi, NGX_POOL_ALIGNMENT, size, log);
    if (p == NULL) {
        return NULL;
    }

    p->d.last = (u_char *) p + sizeof(ngx_pool_t);
    p->d.end = (u_char *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_t);
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->chain = NULL;
    p->large = NULL;
    p->cleanup = NULL;
    p->log = log;

    return p;
}



ngx_int_t __real_ngx_http_parse_header_line(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_uint_t allow_underscores); 
ngx_int_t __wrap_ngx_http_parse_header_line(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_uint_t allow_underscores)
{
    ngx_int_t        rc asm("r12");


    sdrad_enter(NGX_NESTED_DOMAIN); 
    rc = __real_ngx_http_parse_header_line(r,b,allow_underscores); 
    sdrad_exit(); 

    return rc;
}


ngx_int_t __real_ngx_http_parse_request_line(ngx_http_request_t *r, ngx_buf_t *b);
ngx_int_t __wrap_ngx_http_parse_request_line(ngx_http_request_t *r, ngx_buf_t *b)
{
    ngx_int_t               rc asm("r12");
    int buff_size;

    if(start_routine == 0){
        start_routine = 1; 
        pool_copy = __ngx_create_pool(NGX_NESTED_DOMAIN, 4096, NULL);
        header_copy =  ngx_create_temp_buf(pool_copy, 1024);
        sdrad_dprotect(NGX_NESTED_DOMAIN, NGX_DATA_DOMAIN, 0);
    }
       
    buff_size = b->last - b->start; 
    //header_copy =  ngx_create_temp_buf(r->pool, buff_size);
    memcpy(header_copy->start, b->start, buff_size);  
    header_copy->pos = header_copy->start; 
    header_copy->last = header_copy->start + buff_size; 
    r->header_in = header_copy;

    sdrad_enter(NGX_NESTED_DOMAIN); 
    rc = __real_ngx_http_parse_request_line(r, header_copy);
    sdrad_exit(); 

    return rc;
}


void
__ngx_destroy_pool(ngx_pool_t *pool)
{
    ngx_pool_t          *p, *n;
    ngx_pool_large_t    *l;
    ngx_pool_cleanup_t  *c;

    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                           "run cleanup: %p", c);
            c->handler(c->data);
        }
    }

#if (NGX_DEBUG)

    /*
     * we could allocate the pool->log from this pool
     * so we cannot use this log while free()ing the pool
     */

    for (l = pool->large; l; l = l->next) {
        ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0, "free: %p", l->alloc);
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                       "free: %p, unused: %uz", p, p->d.end - p->d.last);

        if (n == NULL) {
            break;
        }
    }

#endif

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            sdrad_free(NGX_DATA_DOMAIN, l->alloc);
        }
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        sdrad_free(NGX_DATA_DOMAIN, p);

        if (n == NULL) {
            break;
        }
    }
}

