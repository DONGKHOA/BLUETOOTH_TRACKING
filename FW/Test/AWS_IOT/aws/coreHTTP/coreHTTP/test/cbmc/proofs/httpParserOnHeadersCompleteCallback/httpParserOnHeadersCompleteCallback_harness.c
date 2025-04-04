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
 * @file httpParserOnHeadersCompleteCallback_harness.c
 * @brief Implements the proof harness for httpParserOnHeadersCompleteCallback function.
 */

#include "http_cbmc_state.h"
#include "llhttp.h"
#include "callback_stubs.h"

int __CPROVER_file_local_core_http_client_c_httpParserOnHeadersCompleteCallback( llhttp_t * pHttpParser );

void httpParserOnHeadersCompleteCallback_harness()
{
    llhttp_t * pHttpParser;
    HTTPParsingContext_t * pParsingContext;
    HTTPResponse_t * pResponse;
    HTTPClient_ResponseHeaderParsingCallback_t headerParserCallback;
    size_t bufferOffset;
    size_t headerOffSet;

    pHttpParser = allocateHttpSendParser( NULL );

    pParsingContext = ( HTTPParsingContext_t * ) ( pHttpParser->data );
    headerParserCallback.onHeaderCallback = onHeaderCallbackStub;

    pResponse = pParsingContext->pResponse;
    pResponse->pHeaderParsingCallback = &headerParserCallback;

    /* The next place to parse must be between the start of the buffer and the
     * end of the buffer. This assumption is an assertion in the function. */
    __CPROVER_assume( bufferOffset < pResponse->bufferLen );
    pParsingContext->pBufferCur = pResponse->pBuffer + bufferOffset;

    /* The response headers MUST have been set prior or be NULL. This assumption
     * is an assertion in the function. */
    if( pResponse->pHeaders != NULL )
    {
        __CPROVER_assume( headerOffSet < bufferOffset );
        pResponse->pHeaders = pResponse->pBuffer + headerOffSet;
    }

    /* This assumption suppresses an overflow error when incrementing pResponse->headerCount. */
    __CPROVER_assume( pResponse->headerCount < SIZE_MAX );

    __CPROVER_file_local_core_http_client_c_httpParserOnHeadersCompleteCallback( pHttpParser );
}
