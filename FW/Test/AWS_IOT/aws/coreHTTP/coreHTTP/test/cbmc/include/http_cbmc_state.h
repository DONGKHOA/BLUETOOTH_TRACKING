/*
 * coreHTTP v3.1.1
 * Copyright (C) 2024 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file http_cbmc_state.h
 * @brief Functions to allocate memory and validate data types for proofs.
 */

#ifndef HTTP_CBMC_STATE_H_
#define HTTP_CBMC_STATE_H_

#include <stdbool.h>
#include <stdint.h>

#include "core_http_client.h"
#include "core_http_client_private.h"
#include "llhttp.h"
#include "transport_interface_stubs.h"

struct NetworkContext
{
    int filler;
};

/**
 * @brief Attains coverage when a variable needs to possibly contain two values.
 */
bool nondet_bool();

/**
 * @brief Calls malloc based on given size or returns NULL for coverage.
 *
 * Implementation of safe malloc which returns NULL if the requested size is 0.
 * The behavior of malloc(0) is platform dependent.
 * It is possible for malloc(0) to return an address without allocating memory.
 *
 * @param[in] size Requested size to malloc.
 *
 * @return Requested memory or NULL.
 */
void * mallocCanFail( size_t size );

/**
 * @brief Allocates an #HTTPRequestHeaders_t object.
 *
 * @param[in] pRequestHeaders #HTTPRequestHeaders_t object to allocate.
 *
 * @return NULL or pointer to allocated #HTTPRequestHeaders_t object.
 */
HTTPRequestHeaders_t * allocateHttpRequestHeaders( HTTPRequestHeaders_t * pRequestHeaders );

/**
 * @brief Validates if a #HTTPRequestHeaders_t object is feasible.
 *
 * @param[in] pRequestHeaders #HTTPRequestHeaders_t object to validate.
 *
 * @return True if #pRequestHeaders is feasible; false otherwise.
 */
bool isValidHttpRequestHeaders( const HTTPRequestHeaders_t * pRequestHeaders );

/**
 * @brief Allocates a #HTTPRequestInfo_t object.
 *
 * @param[in] pRequestInfo #HTTPRequestInfo_t object to allocate.
 *
 * @return NULL or pointer to allocated #HTTPRequestInfo_t object.
 */
HTTPRequestInfo_t * allocateHttpRequestInfo( HTTPRequestInfo_t * pRequestInfo );

/**
 * @brief Validates if a #HTTPRequestInfo_t object is feasible.
 *
 * @param[in] pRequestInfo #HTTPRequestInfo_t object to validate.
 *
 * @return True if #pRequestInfo is feasible; false otherwise.
 */
bool isValidHttpRequestInfo( const HTTPRequestInfo_t * pRequestInfo );

/**
 * @brief Allocates a #HTTPResponse_t object.
 *
 * @param[in] pResponse #HTTPResponse_t object to allocate.
 *
 * @return NULL or pointer to allocated #HTTPResponse_t object.
 */
HTTPResponse_t * allocateHttpResponse( HTTPResponse_t * pResponse );

/**
 * @brief Validates if a #HTTPResponse_t object is feasible.
 *
 * @param[in] pResponse #HTTPResponse_t object to validate.
 *
 * @return True if #HTTPResponse_t is feasible; false otherwise.
 */
bool isValidHttpResponse( const HTTPResponse_t * pResponse );

/**
 * @brief Allocates a #TransportInterface_t object.
 *
 * @param[in] pTransport #TransportInterface_t object to allocate.
 *
 * @return NULL or pointer to allocated #TransportInterface_t object.
 */
TransportInterface_t * allocateTransportInterface( TransportInterface_t * pTransport );

/**
 * @brief Validates if a #TransportInterface_t object is feasible.
 *
 * @param[in] pTransportInterface #TransportInterface_t object to validate.
 *
 * @return True if #pTransportInterface is feasible; false otherwise.
 */
bool isValidTransportInterface( TransportInterface_t * pTransportInterface );

/**
 * @brief Allocate an #llhttp_t object that is valid in the context of the
 * HTTPClient_Send() function.
 *
 * @param[in] pHttpParser #llhttp_t object to allocate.
 *
 * @return NULL or pointer to allocated #llhttp_t object.
 */
llhttp_t * allocateHttpSendParser( llhttp_t * pHttpParser );

/**
 * @brief Allocate an #HTTPParsingContext_t object.
 *
 * @param[in] pHttpParsingContext #HTTPParsingContext_t object to allocate.
 *
 * @return NULL or pointer to allocated #HTTPParsingContext_t object.
 */
HTTPParsingContext_t * allocateHttpSendParsingContext( HTTPParsingContext_t * pHttpParsingContext );

/**
 * @brief Validates if a #HTTPParsingContext_t object is feasible.
 *
 * @param[in] pHttpParsingContext #HTTPParsingContext_t object to validate.
 *
 * @return True if #pHttpParsingContext is feasible; false otherwise.
 */
bool isValidHttpSendParsingContext( const HTTPParsingContext_t * pHttpParsingContext );

/**
 * @brief Allocate an #llhttp_t object that is valid in the context of the
 * HTTPClient_ReadHeader() function.
 *
 * @param[in] pHttpParser #llhttp_t object to allocate.
 *
 * @return NULL or pointer to allocated #llhttp_t object.
 */
llhttp_t * allocateHttpReadHeaderParser( llhttp_t * pHttpParser );

/**
 * @brief Allocate an #findHeaderContext_t object.
 *
 * @param[in] pFindHeaderContext #findHeaderContext_t object to allocate.
 *
 * @return NULL or pointer to allocated #findHeaderContext_t object.
 */
findHeaderContext_t * allocateFindHeaderContext( findHeaderContext_t * pFindHeaderContext );


#endif /* ifndef HTTP_CBMC_STATE_H_ */
