/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Server
 *  ==================
 *  Copyright 2001-2015 Monkey Software LLC <eduardo@monkey.io>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <monkey/mk_iov.h>
#include <monkey/mk_cache.h>
#include <monkey/mk_string.h>
#include <monkey/mk_config.h>
#include <monkey/mk_macros.h>
#include <monkey/mk_utils.h>
#include <monkey/mk_vhost.h>
#include <monkey/mk_tls.h>

#ifndef PTHREAD_TLS
#include <monkey/mk_cache_tls.h>
#endif

pthread_key_t mk_utils_error_key;


/* This function is called when a thread is created */
void mk_cache_worker_init()
{
    char *cache_error;
    mk_ptr_t *p_tmp;

    /* Cache header request -> last modified */
    p_tmp = mk_mem_malloc_z(sizeof(mk_ptr_t));
    p_tmp->data = mk_mem_malloc_z(32);
    p_tmp->len = -1;
    MK_TLS_SET(mk_tls_cache_header_lm, p_tmp);

    /* Cache header request -> content length */
    p_tmp = mk_mem_malloc_z(sizeof(mk_ptr_t));
    p_tmp->data = mk_mem_malloc_z(MK_UTILS_INT2MKP_BUFFER_LEN);
    p_tmp->len = -1;
    MK_TLS_SET(mk_tls_cache_header_cl, p_tmp);

    /* Cache header response -> keep-alive */
    p_tmp = mk_mem_malloc_z(sizeof(mk_ptr_t));
    mk_string_build(&p_tmp->data, &p_tmp->len,
                    "Keep-Alive: timeout=%i, max=",
                    mk_config->keep_alive_timeout);
    MK_TLS_SET(mk_tls_cache_header_ka, p_tmp);

    /* Cache header response -> max=%i */
    p_tmp = mk_mem_malloc_z(sizeof(mk_ptr_t));
    p_tmp->data = mk_mem_malloc_z(64);
    p_tmp->len  = 0;
    MK_TLS_SET(mk_tls_cache_header_ka_max, p_tmp);

    /* Cache iov header struct */
    MK_TLS_SET(mk_tls_cache_iov_header, mk_iov_create(32, 0));

    /* Cache gmtime buffer */
    MK_TLS_SET(mk_tls_cache_gmtime, mk_mem_malloc(sizeof(struct tm)));

    /* Cache the most used text representations of utime2gmt */
    MK_TLS_SET(mk_tls_cache_gmtext,
                  mk_mem_malloc_z(sizeof(struct mk_gmt_cache) * MK_GMT_CACHES));

    /* Cache buffer for strerror_r(2) */
    cache_error = mk_mem_malloc(MK_UTILS_ERROR_SIZE);
    pthread_setspecific(mk_utils_error_key, (void *) cache_error);

    /* Virtual hosts: initialize per thread-vhost data */
    mk_vhost_fdt_worker_init();
}

void mk_cache_worker_exit()
{
    char *cache_error;

    /* Cache header request -> last modified */
    mk_ptr_free(MK_TLS_GET(mk_tls_cache_header_lm));
    mk_mem_free(MK_TLS_GET(mk_tls_cache_header_lm));

    /* Cache header request -> content length */
    mk_ptr_free(MK_TLS_GET(mk_tls_cache_header_cl));
    mk_mem_free(MK_TLS_GET(mk_tls_cache_header_cl));

    /* Cache header response -> keep-alive */
    mk_ptr_free(MK_TLS_GET(mk_tls_cache_header_ka));
    mk_mem_free(MK_TLS_GET(mk_tls_cache_header_ka));

    /* Cache header response -> max=%i */
    mk_ptr_free(MK_TLS_GET(mk_tls_cache_header_ka_max));
    mk_mem_free(MK_TLS_GET(mk_tls_cache_header_ka_max));

    /* Cache iov header struct */
    mk_iov_free(MK_TLS_GET(mk_tls_cache_iov_header));

    /* Cache gmtime buffer */
    mk_mem_free(MK_TLS_GET(mk_tls_cache_gmtime));

    /* Cache the most used text representations of utime2gmt */
    mk_mem_free(MK_TLS_GET(mk_tls_cache_gmtext));

    /* Cache buffer for strerror_r(2) */
    cache_error = pthread_getspecific(mk_utils_error_key);
    mk_mem_free(cache_error);
}
