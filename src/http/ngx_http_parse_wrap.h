/**
 * @file ngx_http_parse_wrap.h
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


#define NGX_NESTED_DOMAIN 3
#define NGX_DATA_DOMAIN 4


int data_domain_flag;

ngx_int_t __real_ngx_http_parse_request_line(ngx_http_request_t *r, ngx_buf_t *b);
ngx_int_t __real_ngx_http_parse_header_line(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_uint_t allow_underscores); 
void * __ngx_memalign(int udi, size_t alignment, size_t size, ngx_log_t *log); 
ngx_pool_t *__ngx_create_pool(int udi, size_t size, ngx_log_t *log); 

