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
 * @file HTTPClient_AddRangeHeader_harness.c
 * @brief Implements the proof harness for HTTPClient_AddRangeHeader function.
 */

#include "core_http_client.h"

#include "http_cbmc_state.h"

void HTTPClient_AddRangeHeader_harness()
{
    HTTPRequestHeaders_t * pRequestHeaders;
    int32_t rangeStartOrlastNbytes;
    int32_t rangeEnd;

    /* Initialize and make assumptions for request headers. */
    pRequestHeaders = allocateHttpRequestHeaders( NULL );
    __CPROVER_assume( isValidHttpRequestHeaders( pRequestHeaders ) );

    HTTPClient_AddRangeHeader( pRequestHeaders, rangeStartOrlastNbytes, rangeEnd );
}
