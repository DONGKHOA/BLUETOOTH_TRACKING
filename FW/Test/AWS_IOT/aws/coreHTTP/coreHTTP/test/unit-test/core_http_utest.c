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

#include <string.h>

#include "unity.h"

/* Include paths for public enums, structures, and macros. */
#include "core_http_client.h"

/* Private includes for internal macros. */
#include "core_http_client_private.h"

/* Include mock implementation of llhttp dependency. */
#include "mock_llhttp.h"

/* Default size for request buffer. */
#define HTTP_TEST_BUFFER_SIZE           ( 100 )

/* Headers data with "\r\n\r\n" terminator to be pre-populated in buffer before
 * call to AddRangeHeader(). */
#define PREEXISTING_HEADER_DATA         "POST / HTTP/1.1 \r\nAuthorization: None\r\n\r\n"
#define PREEXISTING_HEADER_DATA_LEN     ( sizeof( PREEXISTING_HEADER_DATA ) - 1 )

/* Headers data without "\r\n\r\n" terminator to be pre-populated in buffer before
 * call to AddRangeHeader(). */
#define PREEXISTING_REQUEST_LINE        "POST / HTTP/1.1 \r\n"
#define PREEXISTING_REQUEST_LINE_LEN    ( sizeof( PREEXISTING_REQUEST_LINE ) - 1 )

/* Type to store expected headers data. */
typedef struct _headers
{
    uint8_t buffer[ HTTP_TEST_BUFFER_SIZE ];
    size_t dataLen;
} _headers_t;

#define HTTP_METHOD_GET_LEN           ( sizeof( HTTP_METHOD_GET ) - 1 )
#define HTTP_TEST_REQUEST_PATH        "/robots.txt"
#define HTTP_TEST_REQUEST_PATH_LEN    ( sizeof( HTTP_TEST_REQUEST_PATH ) - 1 )
#define HTTP_TEST_HOST_VALUE          "amazon.com"
#define HTTP_TEST_HOST_VALUE_LEN      ( sizeof( HTTP_TEST_HOST_VALUE ) - 1 )
#define HTTP_TEST_REQUEST_LINE   \
    ( HTTP_METHOD_GET " "        \
      HTTP_TEST_REQUEST_PATH " " \
      HTTP_PROTOCOL_VERSION "\r\n" )
#define HTTP_TEST_REQUEST_LINE_LEN    ( sizeof( HTTP_TEST_REQUEST_LINE ) - 1 )

/* Used for format parameter in snprintf(...). */
#define HTTP_TEST_HEADER_FORMAT \
    "%s %s %s\r\n"              \
    "%s: %s\r\n"                \
    "%s: %s\r\n\r\n"

#define HTTP_TEST_HEADER_NO_USER_AGENT_FORMAT \
    "%s %s %s\r\n"                            \
    "%s: %s\r\n\r\n"

#define HTTP_TEST_EXTRA_HEADER_FORMAT \
    "%s %s %s\r\n"                    \
    "%s: %s\r\n"                      \
    "%s: %s\r\n"                      \
    "%s: %s\r\n\r\n"

/* Length of the following template HTTP header.
 *   <HTTP_METHOD_GET> <HTTP_TEST_REQUEST_PATH> <HTTP_PROTOCOL_VERSION> \r\n
 *   <HTTP_USER_AGENT_FIELD>: <HTTP_USER_AGENT_FIELD_LEN> \r\n
 *   <HTTP_HOST_FIELD>: <HTTP_TEST_HOST_VALUE> \r\n
 *   \r\n
 * This is used to initialize the expectedHeader string. */
#define HTTP_TEST_PREFIX_HEADER_LEN                                 \
    ( HTTP_METHOD_GET_LEN + SPACE_CHARACTER_LEN +                   \
      HTTP_TEST_REQUEST_PATH_LEN + SPACE_CHARACTER_LEN +            \
      HTTP_PROTOCOL_VERSION_LEN + HTTP_HEADER_LINE_SEPARATOR_LEN +  \
      HTTP_USER_AGENT_FIELD_LEN + HTTP_HEADER_FIELD_SEPARATOR_LEN + \
      HTTP_USER_AGENT_VALUE_LEN + HTTP_HEADER_LINE_SEPARATOR_LEN +  \
      HTTP_HOST_FIELD_LEN + HTTP_HEADER_FIELD_SEPARATOR_LEN +       \
      HTTP_TEST_HOST_VALUE_LEN + HTTP_HEADER_LINE_SEPARATOR_LEN +   \
      HTTP_HEADER_LINE_SEPARATOR_LEN )

/* Length of the following template HTTP header.
 *   <HTTP_METHOD_GET> <HTTP_TEST_REQUEST_PATH> <HTTP_PROTOCOL_VERSION> \r\n
 *   <HTTP_HOST_FIELD>: <HTTP_TEST_HOST_VALUE> \r\n
 *   \r\n
 * This is used to initialize the expectedHeader string. */
#define HTTP_TEST_PREFIX_HEADER_NO_USER_AGENT_LEN                  \
    ( HTTP_METHOD_GET_LEN + SPACE_CHARACTER_LEN +                  \
      HTTP_TEST_REQUEST_PATH_LEN + SPACE_CHARACTER_LEN +           \
      HTTP_PROTOCOL_VERSION_LEN + HTTP_HEADER_LINE_SEPARATOR_LEN + \
      HTTP_HOST_FIELD_LEN + HTTP_HEADER_FIELD_SEPARATOR_LEN +      \
      HTTP_TEST_HOST_VALUE_LEN + HTTP_HEADER_LINE_SEPARATOR_LEN +  \
      HTTP_HEADER_LINE_SEPARATOR_LEN )

/* Add 1 because snprintf(...) writes a null byte at the end. */
#define HTTP_TEST_INITIALIZED_HEADER_BUFFER_LEN \
    ( HTTP_TEST_PREFIX_HEADER_LEN + 1 )

/* Template HTTP header fields and values. */
#define HTTP_TEST_HEADER_FIELD               "Authorization"
#define HTTP_TEST_HEADER_FIELD_LEN           ( sizeof( HTTP_TEST_HEADER_FIELD ) - 1 )
#define HTTP_TEST_HEADER_VALUE               "None"
#define HTTP_TEST_HEADER_VALUE_LEN           ( sizeof( HTTP_TEST_HEADER_VALUE ) - 1 )
/* Template for first line of HTTP header. */
#define HTTP_TEST_HEADER_REQUEST_LINE        "GET / HTTP/1.1 \r\n"
#define HTTP_TEST_HEADER_REQUEST_LINE_LEN    ( sizeof( HTTP_TEST_HEADER_REQUEST_LINE ) - 1 )
#define HTTP_REQUEST_HEADERS_INITIALIZER     { 0 }
/* Template for snprintf(...) strings. */
#define HTTP_TEST_SINGLE_HEADER_FORMAT       "%s%s: %s\r\n\r\n"
#define HTTP_TEST_DOUBLE_HEADER_FORMAT       "%s%s: %s\r\n%s: %s\r\n\r\n"

/* Length of the following template HTTP header.
 *   <HTTP_TEST_HEADER_REQUEST_LINE> \r\n
 *   <HTTP_TEST_HEADER_FIELD>: <HTTP_TEST_HEADER_VALUE> \r\n
 *   \r\n
 * This is used to initialize the expectedHeader string. */
#define HTTP_TEST_SINGLE_HEADER_LEN       \
    ( HTTP_TEST_HEADER_REQUEST_LINE_LEN + \
      HTTP_TEST_HEADER_FIELD_LEN +        \
      HTTP_HEADER_FIELD_SEPARATOR_LEN +   \
      HTTP_TEST_HEADER_VALUE_LEN +        \
      HTTP_HEADER_LINE_SEPARATOR_LEN +    \
      HTTP_HEADER_LINE_SEPARATOR_LEN )

/* The longest possible header used for these unit tests. */
#define HTTP_TEST_DOUBLE_HEADER_LEN     \
    ( HTTP_TEST_SINGLE_HEADER_LEN +     \
      HTTP_TEST_HEADER_FIELD_LEN +      \
      HTTP_HEADER_FIELD_SEPARATOR_LEN + \
      HTTP_TEST_HEADER_VALUE_LEN +      \
      HTTP_HEADER_LINE_SEPARATOR_LEN )

#define HTTP_TEST_DOUBLE_HEADER_BUFFER_LEN \
    ( HTTP_TEST_DOUBLE_HEADER_LEN + 1 )

/* Template HTTP response for testing HTTPClient_ReadHeader API. */
static const char * pTestResponse = "HTTP/1.1 200 OK\r\n"
                                    "test-header0: test-value0\r\n"
                                    "test-header1: test-value1\r\n"
                                    "test-header2: test-value2\r\n"
                                    "header_not_in_buffer: test-value3\r\n"
                                    "\r\n";

/* HTTP response for testing HTTPClient_ReadHeader API. This response contains
 * an empty value. */
static const char * pTestResponseEmptyValue = "HTTP/1.1 200 OK\r\n"
                                              "test-header0: test-value0\r\n"
                                              "test-header1: \r\n"
                                              "test-header2: test-value2\r\n"
                                              "\r\n";

/* Template HTTP response for testing HTTPClient_ReadHeader API
 * with special characters. */
static const char * pTestResponseSpecialCharacter = "HTTP/1.1 200 OK\r\n"
                                                    "test-header0: test-value0\r\n"
                                                    "test-header1: test-value1\r\n"
                                                    "test-header2: test|value2\r\n"
                                                    "header{_not_in|buff}: test-value3\r\n"
                                                    "\r\n";

#define HEADER_INVALID_PARAMS                 "Header"
#define HEADER_INVALID_PARAMS_LEN             ( sizeof( HEADER_INVALID_PARAMS ) - 1 )

#define HEADER_IN_BUFFER                      "teSt-hEader1"
#define HEADER_IN_BUFFER_LEN                  ( sizeof( HEADER_IN_BUFFER ) - 1 )

#define HEADER_NOT_IN_BUFFER                  "header-not-in-buffer"
#define HEADER_NOT_IN_BUFFER_LEN              ( sizeof( HEADER_NOT_IN_BUFFER ) - 1 )

#define HEADER_WITH_SPECIAL_CHARACTERS        "header{_not-in|buff}"
#define HEADER_WITH_SPECIAL_CHARACTERS_LEN    ( sizeof( HEADER_WITH_SPECIAL_CHARACTERS ) - 1 )

/* File-scoped Global variables */
static HTTPStatus_t retCode = HTTPSuccess;
static uint8_t testBuffer[ HTTP_TEST_BUFFER_SIZE ] = { 0 };
static HTTPRequestHeaders_t testHeaders = { 0 };
static _headers_t expectedHeaders = { 0 };
static int testRangeStart = 0;
static int testRangeEnd = 0;
static const char * pValueLoc = NULL;
static size_t valueLen = 0U;
static HTTPResponse_t testResponse = { 0 };
static const size_t headerFieldInRespLoc = 44;
static const size_t headerFieldInRespLen = sizeof( "test-header1" ) - 1U;
static const size_t otherHeaderFieldInRespLoc = 98;
static const size_t otherHeaderFieldInRespLen = sizeof( "header_not_in_buffer" ) - 1U;
static const size_t headerValInRespLoc = 58;
static const size_t headerValInRespLen = sizeof( "test-value1" ) - 1U;
static llhttp_t * pCapturedParser = NULL;
static llhttp_settings_t * pCapturedSettings = NULL;
static const char * pExpectedBuffer = NULL;
static size_t expectedBufferSize = 0U;
static uint8_t invokeHeaderFieldCallback = 0U;
static const char * pFieldLocToReturn = NULL;
static size_t fieldLenToReturn = 0U;
static uint8_t invokeHeaderValueCallback = 0U;
static const char * pValueLocToReturn = NULL;
static size_t valueLenToReturn = 0U;
static int expectedValCbRetVal = 0;
static uint8_t invokeHeaderCompleteCallback = 0U;
static unsigned int parserErrNo = 0;

/* ============================ Helper Functions ============================== */

/**
 * @brief Callback that is passed to the mock of llhttp_init function
 * to set test expectations on input arguments sent by the HTTP API function under
 * test.
 */
void parserInitExpectationCb( llhttp_t * parser,
                              enum llhttp_type type,
                              const llhttp_settings_t * settings,
                              int cmock_num_calls )
{
    /* Disable unused parameter warning. */
    ( void ) cmock_num_calls;

    TEST_ASSERT_NOT_NULL( parser );
    pCapturedParser = parser;
    TEST_ASSERT_EQUAL( pCapturedSettings, settings );
    /* Set the settings member expected by calls to llhttp_execute(). */
    parser->settings = ( llhttp_settings_t * ) settings;

    TEST_ASSERT_EQUAL( HTTP_RESPONSE, type );
}

/**
 * @brief Callback that is passed to the mock of llhttp_settings_init function
 * to set test expectations on input arguments sent by the HTTP API function under
 * test.
 */
void parserSettingsInitExpectationCb( llhttp_settings_t * settings,
                                      int cmock_num_calls )
{
    /* Disable unused parameter warning. */
    ( void ) cmock_num_calls;

    TEST_ASSERT_NOT_NULL( settings );
    pCapturedSettings = settings;
}

/**
 * @brief Callback that is passed to the mock of llhttp_execute() function
 * to set test expectations on input arguments, and inject behavior of invoking
 * llhttp callbacks depending on test-case specific configuration of the
 * function.
 */
llhttp_errno_t parserExecuteExpectationsCb( llhttp_t * parser,
                                            const char * data,
                                            size_t len,
                                            int cmock_num_calls )
{
    /* Disable unused parameter warning. */
    ( void ) cmock_num_calls;

    TEST_ASSERT_EQUAL( pCapturedParser, parser );
    TEST_ASSERT_NOT_NULL( parser );
    TEST_ASSERT_EQUAL( pCapturedSettings, parser->settings );

    TEST_ASSERT_EQUAL( expectedBufferSize, len );
    TEST_ASSERT_EQUAL( pExpectedBuffer, data );

    if( invokeHeaderFieldCallback == 1U )
    {
        TEST_ASSERT_EQUAL( LLHTTP_CONTINUE_PARSING,
                           ( ( llhttp_settings_t * ) ( parser->settings ) )->on_header_field( parser,
                                                                                              pFieldLocToReturn,
                                                                                              fieldLenToReturn ) );
    }

    if( invokeHeaderValueCallback == 1U )
    {
        TEST_ASSERT_EQUAL( expectedValCbRetVal,
                           ( ( llhttp_settings_t * ) ( parser->settings ) )->on_header_value( parser,
                                                                                              pValueLocToReturn,
                                                                                              valueLenToReturn ) );
    }

    if( invokeHeaderCompleteCallback == 1U )
    {
        TEST_ASSERT_EQUAL( LLHTTP_STOP_PARSING_NO_HEADER,
                           ( ( llhttp_settings_t * ) ( parser->settings ) )->on_headers_complete( parser ) );
    }

    /* Set the error value in the parser. */
    parser->error = parserErrNo;
    return parserErrNo;
}

/**
 * @brief Fills the test input buffer and expectation buffers with pre-existing data
 * before calling the API function under test.
 */
static void setupBuffersWithPreexistingHeader( HTTPRequestHeaders_t * testRequestHeaders,
                                               uint8_t * testBuffer,
                                               size_t bufferSize,
                                               _headers_t * expectedHeaders,
                                               const char * preexistingData )
{
    size_t dataLen = strlen( preexistingData );
    int numBytes = 0;

    testRequestHeaders->pBuffer = testBuffer;
    testRequestHeaders->bufferLen = bufferSize;

    numBytes = snprintf( ( char * ) testRequestHeaders->pBuffer,
                         bufferSize,
                         "%s",
                         preexistingData );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( bufferSize, ( size_t ) numBytes );
    testRequestHeaders->headersLen = dataLen;

    /* Fill the same data in the expected buffer as HTTPClient_AddRangeHeaders()
     * is not expected to change it. */
    memcpy( expectedHeaders->buffer, testRequestHeaders->pBuffer,
            testRequestHeaders->headersLen );
    expectedHeaders->dataLen = testRequestHeaders->headersLen;
}

/**
 * @brief Common utility for adding the expected range string for a AddRangeRequest test case
 * in the expectation buffer.
 */
static void addRangeToExpectedHeaders( _headers_t * expectedHeaders,
                                       const char * expectedRange,
                                       bool terminatorExists )
{
    size_t expectedRangeLen = HTTP_RANGE_REQUEST_HEADER_FIELD_LEN +
                              HTTP_HEADER_FIELD_SEPARATOR_LEN +
                              HTTP_RANGE_REQUEST_HEADER_VALUE_PREFIX_LEN +
                              strlen( expectedRange ) +
                              2 * HTTP_HEADER_LINE_SEPARATOR_LEN;

    int numBytes =
        snprintf( ( char * ) expectedHeaders->buffer +
                  expectedHeaders->dataLen -
                  ( terminatorExists ? HTTP_HEADER_LINE_SEPARATOR_LEN : 0 ),
                  sizeof( expectedHeaders->buffer ) - expectedHeaders->dataLen,
                  "%s%s%s%s\r\n\r\n",
                  HTTP_RANGE_REQUEST_HEADER_FIELD,
                  HTTP_HEADER_FIELD_SEPARATOR,
                  HTTP_RANGE_REQUEST_HEADER_VALUE_PREFIX,
                  expectedRange );

    /* Make sure that the Range request was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( sizeof( expectedHeaders->buffer ), ( size_t ) numBytes );

    expectedHeaders->dataLen += expectedRangeLen -
                                ( terminatorExists ? HTTP_HEADER_LINE_SEPARATOR_LEN : 0 );
}

/* ============================ UNITY FIXTURES ============================== */

/* Called before each test method. */
void setUp()
{
    testResponse.pBuffer = ( uint8_t * ) &pTestResponse[ 0 ];
    testResponse.bufferLen = strlen( pTestResponse );

    /* Configure the llhttp mocks with their callbacks. */
    llhttp_settings_init_AddCallback( parserSettingsInitExpectationCb );
    llhttp_init_AddCallback( parserInitExpectationCb );
    llhttp_execute_AddCallback( parserExecuteExpectationsCb );

    /* Ignore the calls to llhttp_get_error_reason. */
    llhttp_errno_name_IgnoreAndReturn( "Mocked HTTP Parser Status" );
    llhttp_get_error_reason_IgnoreAndReturn( "Mocked HTTP Parser Status" );
}

/* Called after each test method. */
void tearDown()
{
    retCode = HTTPSuccess;
    memset( &testHeaders, 0, sizeof( testHeaders ) );
    memset( testBuffer, 0, sizeof( testBuffer ) );
    memset( &expectedHeaders, 0, sizeof( expectedHeaders ) );
    memset( &testResponse,
            0,
            sizeof( testResponse ) );
    testResponse.pBuffer = testBuffer;
    testResponse.bufferLen = strlen( pTestResponse );
    pValueLoc = NULL;
    valueLen = 0U;
    pValueLoc = NULL;
    valueLen = 0U;
    pExpectedBuffer = &pTestResponse[ 0 ];
    expectedBufferSize = strlen( pTestResponse );
    invokeHeaderFieldCallback = 0U;
    pFieldLocToReturn = NULL;
    fieldLenToReturn = 0U;
    invokeHeaderValueCallback = 0U;
    pValueLocToReturn = NULL;
    expectedValCbRetVal = 0;
    valueLenToReturn = 0U;
    invokeHeaderCompleteCallback = 0U;
}

/* Called at the beginning of the whole suite. */
void suiteSetUp()
{
}

/* Called at the end of the whole suite. */
int suiteTearDown( int numFailures )
{
    return numFailures;
}

/* ============== Testing HTTPClient_InitializeRequestHeaders =============== */

/**
 * @brief Initialize pRequestInfo with test-defined macros.
 *
 * @param[in] pRequestInfo Initial request header configurations.
 */
static void setupRequestInfo( HTTPRequestInfo_t * pRequestInfo )
{
    pRequestInfo->pMethod = HTTP_METHOD_GET;
    pRequestInfo->methodLen = HTTP_METHOD_GET_LEN;
    pRequestInfo->pPath = HTTP_TEST_REQUEST_PATH;
    pRequestInfo->pathLen = HTTP_TEST_REQUEST_PATH_LEN;
    pRequestInfo->pHost = HTTP_TEST_HOST_VALUE;
    pRequestInfo->hostLen = HTTP_TEST_HOST_VALUE_LEN;
    pRequestInfo->reqFlags = 0;
}

/**
 * @brief Initialize pRequestHeaders with static buffer.
 *
 * @param[in] pRequestHeaders Request header buffer information.
 */
static void setupBuffer( HTTPRequestHeaders_t * pRequestHeaders )
{
    pRequestHeaders->pBuffer = testBuffer;
    pRequestHeaders->bufferLen = sizeof( testBuffer );
}

/**
 * @brief Test happy path with zero-initialized requestHeaders and requestInfo.
 */
void test_Http_InitializeRequestHeaders_Happy_Path()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    HTTPRequestInfo_t requestInfo = { 0 };
    int numBytes = 0;

    setupRequestInfo( &requestInfo );
    expectedHeaders.dataLen = HTTP_TEST_PREFIX_HEADER_LEN;
    setupBuffer( &requestHeaders );

    /* Happy Path testing. */
    numBytes = snprintf( ( char * ) expectedHeaders.buffer, sizeof( expectedHeaders.buffer ),
                         HTTP_TEST_HEADER_FORMAT,
                         HTTP_METHOD_GET, HTTP_TEST_REQUEST_PATH,
                         HTTP_PROTOCOL_VERSION,
                         HTTP_USER_AGENT_FIELD, HTTP_USER_AGENT_VALUE,
                         HTTP_HOST_FIELD, HTTP_TEST_HOST_VALUE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( sizeof( expectedHeaders.buffer ), ( size_t ) numBytes );

    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPSuccess, httpStatus );
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, requestHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer, requestHeaders.pBuffer,
                              expectedHeaders.dataLen );
}

/**
 * @brief Test happy path with HTTP_REQUEST_NO_USER_AGENT_FLAG.
 */
void test_Http_InitializeRequestHeaders_no_user_agent_flag()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    HTTPRequestInfo_t requestInfo = { 0 };
    int numBytes = 0;

    setupRequestInfo( &requestInfo );
    requestInfo.reqFlags |= HTTP_REQUEST_NO_USER_AGENT_FLAG;

    expectedHeaders.dataLen = HTTP_TEST_PREFIX_HEADER_NO_USER_AGENT_LEN;
    setupBuffer( &requestHeaders );

    /* Happy Path testing. */
    numBytes = snprintf( ( char * ) expectedHeaders.buffer, sizeof( expectedHeaders.buffer ),
                         HTTP_TEST_HEADER_NO_USER_AGENT_FORMAT,
                         HTTP_METHOD_GET, HTTP_TEST_REQUEST_PATH,
                         HTTP_PROTOCOL_VERSION,
                         HTTP_HOST_FIELD, HTTP_TEST_HOST_VALUE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( sizeof( expectedHeaders.buffer ), ( size_t ) numBytes );

    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPSuccess, httpStatus );
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, requestHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer, requestHeaders.pBuffer,
                              expectedHeaders.dataLen );
}

/**
 * @brief Test NULL parameters, following order of else-if blocks in the HTTP library.
 */
void test_Http_InitializeRequestHeaders_Invalid_Params()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    HTTPRequestInfo_t requestInfo = { 0 };

    /* Test NULL parameters, following order of else-if blocks. */
    httpStatus = HTTPClient_InitializeRequestHeaders( NULL, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );

    /* TEST requestInfo.pBuffer == NULL */
    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );
    requestHeaders.pBuffer = testBuffer;
    requestHeaders.bufferLen = HTTP_TEST_INITIALIZED_HEADER_BUFFER_LEN;

    /* Test requestInfo == NULL. */
    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, NULL );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );

    /* Test requestInfo.pMethod == NULL. */
    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );
    requestInfo.pMethod = HTTP_METHOD_GET;

    /* Test requestInfo.pHost == NULL. */
    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );

    /* Test requestInfo.methodLen == 0. */
    requestInfo.pHost = HTTP_TEST_HOST_VALUE;
    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );

    /* Test requestInfo.hostLen == 0. */
    requestInfo.methodLen = HTTP_METHOD_GET_LEN;
    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );
}

/**
 * @brief Test default path "/" if path == NULL. Also, check that the "Connection"
 * header is set to "keep-alive" when HTTP_REQUEST_KEEP_ALIVE_FLAG in requestHeaders
 * is activated.
 */
void test_Http_InitializeRequestHeaders_ReqInfo()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    HTTPRequestInfo_t requestInfo = { 0 };
    int numBytes = 0;
    size_t connectionKeepAliveHeaderLen = HTTP_CONNECTION_FIELD_LEN +
                                          HTTP_HEADER_FIELD_SEPARATOR_LEN +
                                          HTTP_CONNECTION_KEEP_ALIVE_VALUE_LEN +
                                          HTTP_HEADER_LINE_SEPARATOR_LEN;

    expectedHeaders.dataLen = HTTP_TEST_PREFIX_HEADER_LEN -
                              HTTP_TEST_REQUEST_PATH_LEN +
                              HTTP_EMPTY_PATH_LEN +
                              connectionKeepAliveHeaderLen;

    setupRequestInfo( &requestInfo );
    setupBuffer( &requestHeaders );

    requestInfo.pPath = NULL;
    requestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;
    numBytes = snprintf( ( char * ) expectedHeaders.buffer, sizeof( expectedHeaders.buffer ),
                         HTTP_TEST_EXTRA_HEADER_FORMAT,
                         HTTP_METHOD_GET, HTTP_EMPTY_PATH,
                         HTTP_PROTOCOL_VERSION,
                         HTTP_USER_AGENT_FIELD, HTTP_USER_AGENT_VALUE,
                         HTTP_HOST_FIELD, HTTP_TEST_HOST_VALUE,
                         HTTP_CONNECTION_FIELD, HTTP_CONNECTION_KEEP_ALIVE_VALUE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( sizeof( expectedHeaders.buffer ), ( size_t ) numBytes );

    requestHeaders.pBuffer = testBuffer;
    requestHeaders.bufferLen = expectedHeaders.dataLen;
    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPSuccess, httpStatus );
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, requestHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer, requestHeaders.pBuffer,
                              expectedHeaders.dataLen );

    /* Repeat the test above but with length of path == 0 for coverage. */
    requestInfo.pPath = HTTP_EMPTY_PATH;
    requestInfo.pathLen = 0;
    requestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;
    numBytes = snprintf( ( char * ) expectedHeaders.buffer, sizeof( expectedHeaders.buffer ),
                         HTTP_TEST_EXTRA_HEADER_FORMAT,
                         HTTP_METHOD_GET, HTTP_EMPTY_PATH,
                         HTTP_PROTOCOL_VERSION,
                         HTTP_USER_AGENT_FIELD, HTTP_USER_AGENT_VALUE,
                         HTTP_HOST_FIELD, HTTP_TEST_HOST_VALUE,
                         HTTP_CONNECTION_FIELD, HTTP_CONNECTION_KEEP_ALIVE_VALUE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( sizeof( expectedHeaders.buffer ), ( size_t ) numBytes );

    requestHeaders.pBuffer = testBuffer;
    requestHeaders.bufferLen = expectedHeaders.dataLen;
    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPSuccess, httpStatus );
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, requestHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer, requestHeaders.pBuffer,
                              expectedHeaders.dataLen );
}

/**
 * @brief Test HTTPInsufficientMemory from having requestHeaders.bufferLen less than
 * what is required to fit HTTP_TEST_REQUEST_LINE.
 */
void test_Http_InitializeRequestHeaders_Insufficient_Memory()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    HTTPRequestInfo_t requestInfo = { 0 };

    expectedHeaders.dataLen = HTTP_TEST_PREFIX_HEADER_LEN;

    setupRequestInfo( &requestInfo );
    setupBuffer( &requestHeaders );

    requestHeaders.bufferLen = HTTP_TEST_REQUEST_LINE_LEN - 1;

    httpStatus = HTTPClient_InitializeRequestHeaders( &requestHeaders, &requestInfo );
    TEST_ASSERT_EQUAL( HTTPInsufficientMemory, httpStatus );
    TEST_ASSERT_TRUE( strncmp( ( char * ) requestHeaders.pBuffer,
                               HTTP_TEST_REQUEST_LINE,
                               HTTP_TEST_REQUEST_LINE_LEN ) != 0 );
}

/* ===================== Testing HTTPClient_AddHeader ======================= */

/**
 * @brief Prefill the user buffer with HTTP_TEST_HEADER_REQUEST_LINE and call
 * HTTPClient_AddHeader using HTTP_TEST_HEADER_FIELD and HTTP_TEST_HEADER_VALUE.
 */
void test_Http_AddHeader_Happy_Path()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    int numBytes = 0;

    setupBuffer( &requestHeaders );

    /* Add 1 because snprintf(...) writes a null byte at the end. */
    numBytes = snprintf( ( char * ) expectedHeaders.buffer,
                         sizeof( expectedHeaders.buffer ),
                         HTTP_TEST_SINGLE_HEADER_FORMAT,
                         HTTP_TEST_HEADER_REQUEST_LINE,
                         HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_VALUE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( sizeof( expectedHeaders.buffer ), ( size_t ) numBytes );
    expectedHeaders.dataLen = HTTP_TEST_SINGLE_HEADER_LEN;

    /* Set parameters for requestHeaders. */
    numBytes = snprintf( ( char * ) requestHeaders.pBuffer,
                         HTTP_TEST_HEADER_REQUEST_LINE_LEN + 1,
                         HTTP_TEST_HEADER_REQUEST_LINE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( requestHeaders.bufferLen, ( size_t ) numBytes );
    /* We correctly set headersLen after writing request line to requestHeaders.pBuffer. */
    requestHeaders.headersLen = HTTP_TEST_HEADER_REQUEST_LINE_LEN;

    /* Run the method to test. */
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_FIELD_LEN,
                                       HTTP_TEST_HEADER_VALUE, HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, requestHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              requestHeaders.pBuffer, expectedHeaders.dataLen );
    TEST_ASSERT_EQUAL( HTTPSuccess, httpStatus );
}

/**
 * @brief Test invalid parameters, following order of else-if blocks in the HTTP library.
 */
void test_Http_AddHeader_Invalid_Params()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };

    /* Test a NULL request headers interface. */
    httpStatus = HTTPClient_AddHeader( NULL,
                                       HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_FIELD_LEN,
                                       HTTP_TEST_HEADER_VALUE, HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );

    /* Test a NULL pBuffer member of request headers. */
    requestHeaders.pBuffer = NULL;
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_FIELD_LEN,
                                       HTTP_TEST_HEADER_VALUE, HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );

    /* Test NULL header field. */
    requestHeaders.pBuffer = testBuffer;
    requestHeaders.bufferLen = HTTP_TEST_DOUBLE_HEADER_LEN;
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       NULL, HTTP_TEST_HEADER_FIELD_LEN,
                                       HTTP_TEST_HEADER_VALUE, HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );

    /* Test NULL header value. */
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_FIELD_LEN,
                                       NULL, HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );

    /* Test that fieldLen > 0. */
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD, 0,
                                       HTTP_TEST_HEADER_VALUE, HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );

    /* Test that valueLen > 0. */
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_FIELD_LEN,
                                       HTTP_TEST_HEADER_VALUE, 0 );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );

    /* Test that requestHeaders.headersLen <= requestHeaders.bufferLen. */
    requestHeaders.headersLen = requestHeaders.bufferLen + 1;
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_FIELD_LEN,
                                       HTTP_TEST_HEADER_VALUE, HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, httpStatus );
}

/**
 * @brief Test adding extra header with sufficient memory.
 */
void test_Http_AddHeader_Extra_Header_Sufficient_Memory()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    int numBytes = 0;

    setupBuffer( &requestHeaders );

    /* Add 1 because snprintf(...) writes a null byte at the end. */
    numBytes = snprintf( ( char * ) expectedHeaders.buffer,
                         sizeof( expectedHeaders.buffer ),
                         HTTP_TEST_DOUBLE_HEADER_FORMAT,
                         HTTP_TEST_HEADER_REQUEST_LINE,
                         HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_VALUE,
                         HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_VALUE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( sizeof( expectedHeaders.buffer ), ( size_t ) numBytes );
    expectedHeaders.dataLen = HTTP_TEST_DOUBLE_HEADER_LEN;

    /* Prefill the buffer with a request line and header. */
    numBytes = snprintf( ( char * ) requestHeaders.pBuffer,
                         HTTP_TEST_SINGLE_HEADER_LEN + 1,
                         HTTP_TEST_SINGLE_HEADER_FORMAT,
                         HTTP_TEST_HEADER_REQUEST_LINE,
                         HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_VALUE );
    TEST_ASSERT_EQUAL( HTTP_TEST_SINGLE_HEADER_LEN, numBytes );
    requestHeaders.headersLen = HTTP_TEST_SINGLE_HEADER_LEN;
    requestHeaders.bufferLen = expectedHeaders.dataLen;

    /* Run the method to test. */
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD,
                                       HTTP_TEST_HEADER_FIELD_LEN,
                                       HTTP_TEST_HEADER_VALUE,
                                       HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, requestHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              requestHeaders.pBuffer, expectedHeaders.dataLen );
    TEST_ASSERT_EQUAL( HTTPSuccess, httpStatus );
}

/**
 * @brief Test adding extra header with insufficient memory.
 */
void test_Http_AddHeader_Extra_Header_Insufficient_Memory()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    int numBytes = 0;

    setupBuffer( &requestHeaders );

    /* Add 1 because snprintf(...) writes a null byte at the end. */
    numBytes = snprintf( ( char * ) expectedHeaders.buffer,
                         sizeof( expectedHeaders.buffer ),
                         HTTP_TEST_SINGLE_HEADER_FORMAT,
                         HTTP_TEST_HEADER_REQUEST_LINE,
                         HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_VALUE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( sizeof( expectedHeaders.buffer ), ( size_t ) numBytes );
    expectedHeaders.dataLen = HTTP_TEST_SINGLE_HEADER_LEN;

    /* Prefill the buffer with a request line and header. */
    numBytes = snprintf( ( char * ) requestHeaders.pBuffer,
                         HTTP_TEST_SINGLE_HEADER_LEN + 1,
                         HTTP_TEST_SINGLE_HEADER_FORMAT,
                         HTTP_TEST_HEADER_REQUEST_LINE,
                         HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_VALUE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( requestHeaders.bufferLen, ( size_t ) numBytes );
    requestHeaders.headersLen = HTTP_TEST_SINGLE_HEADER_LEN;
    requestHeaders.bufferLen = requestHeaders.headersLen;

    /* Run the method to test. */
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD,
                                       HTTP_TEST_HEADER_FIELD_LEN,
                                       HTTP_TEST_HEADER_VALUE,
                                       HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, requestHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              requestHeaders.pBuffer, expectedHeaders.dataLen );
    TEST_ASSERT_EQUAL( HTTPInsufficientMemory, httpStatus );
}

/**
 * @brief Test HTTPInsufficientMemory error from having buffer size less than
 * what is required to fit a single HTTP header.
 */
void test_Http_AddHeader_Single_Header_Insufficient_Memory()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    int numBytes = 0;

    setupBuffer( &requestHeaders );

    /* Add 1 because snprintf(...) writes a null byte at the end. */
    numBytes = snprintf( ( char * ) testBuffer,
                         HTTP_TEST_HEADER_REQUEST_LINE_LEN + 1,
                         HTTP_TEST_HEADER_REQUEST_LINE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    TEST_ASSERT_LESS_THAN( sizeof( testBuffer ), ( size_t ) numBytes );
    requestHeaders.headersLen = HTTP_TEST_HEADER_REQUEST_LINE_LEN;
    requestHeaders.pBuffer = testBuffer;
    requestHeaders.bufferLen = HTTP_TEST_SINGLE_HEADER_LEN - 1;

    /* Run the method to test. */
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD,
                                       HTTP_TEST_HEADER_FIELD_LEN,
                                       HTTP_TEST_HEADER_VALUE,
                                       HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( HTTPInsufficientMemory, httpStatus );
}

/**
 * @brief Test adding invalid header fields.
 */
void test_Http_AddHeader_Invalid_Fields()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    int numBytes = 0;

    const char * colonInField = "head:er-field";
    const char * linefeedInField = "head\ner-field";
    const char * carriageReturnInField = "head\rer-field";

    setupBuffer( &requestHeaders );

    /* Set parameters for requestHeaders. */
    numBytes = snprintf( ( char * ) requestHeaders.pBuffer,
                         HTTP_TEST_HEADER_REQUEST_LINE_LEN + 1,
                         HTTP_TEST_HEADER_REQUEST_LINE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );

    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       colonInField, strlen( colonInField ),
                                       HTTP_TEST_HEADER_VALUE, HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( HTTPSecurityAlertInvalidCharacter, httpStatus );

    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       linefeedInField, strlen( linefeedInField ),
                                       HTTP_TEST_HEADER_VALUE, HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( HTTPSecurityAlertInvalidCharacter, httpStatus );

    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       carriageReturnInField, strlen( carriageReturnInField ),
                                       HTTP_TEST_HEADER_VALUE, HTTP_TEST_HEADER_VALUE_LEN );
    TEST_ASSERT_EQUAL( HTTPSecurityAlertInvalidCharacter, httpStatus );
}

/**
 * @brief Test adding invalid header values.
 */
void test_Http_AddHeader_Invalid_Values()
{
    HTTPStatus_t httpStatus = HTTPSuccess;
    HTTPRequestHeaders_t requestHeaders = { 0 };
    int numBytes = 0;

    const char * colonInValue = "head:er-value";
    const char * linefeedInValue = "head\ner-Value";
    const char * carriageReturnInValue = "head\rer-Value";

    setupBuffer( &requestHeaders );

    /* Test that a colon in the value is OK. */

    /* Add 1 because snprintf(...) writes a null byte at the end. */
    numBytes = snprintf( ( char * ) expectedHeaders.buffer,
                         sizeof( expectedHeaders.buffer ),
                         HTTP_TEST_SINGLE_HEADER_FORMAT,
                         HTTP_TEST_HEADER_REQUEST_LINE,
                         HTTP_TEST_HEADER_FIELD, colonInValue );

    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );
    expectedHeaders.dataLen = numBytes;

    /* Set parameters for requestHeaders. */
    numBytes = snprintf( ( char * ) requestHeaders.pBuffer,
                         HTTP_TEST_HEADER_REQUEST_LINE_LEN + 1,
                         HTTP_TEST_HEADER_REQUEST_LINE );
    /* Make sure that the entire pre-existing data was printed to the buffer. */
    TEST_ASSERT_GREATER_THAN( 0, numBytes );

    /* We correctly set headersLen after writing request line to requestHeaders.pBuffer. */
    requestHeaders.headersLen = HTTP_TEST_HEADER_REQUEST_LINE_LEN;

    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_FIELD_LEN,
                                       colonInValue, strlen( colonInValue ) );
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, requestHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              requestHeaders.pBuffer, expectedHeaders.dataLen );
    TEST_ASSERT_EQUAL( HTTPSuccess, httpStatus );

    /* Now test invalid character cases. */
    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_FIELD_LEN,
                                       linefeedInValue, strlen( linefeedInValue ) );
    TEST_ASSERT_EQUAL( HTTPSecurityAlertInvalidCharacter, httpStatus );

    httpStatus = HTTPClient_AddHeader( &requestHeaders,
                                       HTTP_TEST_HEADER_FIELD, HTTP_TEST_HEADER_FIELD_LEN,
                                       carriageReturnInValue, strlen( carriageReturnInValue ) );
    TEST_ASSERT_EQUAL( HTTPSecurityAlertInvalidCharacter, httpStatus );
}

/* ============== Testing HTTPClient_AddRangeHeader ================== */

/**
 * @brief Testing with invalid parameter inputs.
 */
void test_Http_AddRangeHeader_Invalid_Params( void )
{
    /* Request header parameter is NULL. */
    tearDown();
    retCode = HTTPClient_AddRangeHeader( NULL,
                                         0 /* rangeStart */,
                                         0 /* rangeEnd */ );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );

    /* Underlying buffer is NULL in request headers. */
    tearDown();
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         0 /* rangeStart */,
                                         0 /* rangeEnd */ );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );

    /* Request Header Size is zero. */
    tearDown();
    testHeaders.pBuffer = &testBuffer[ 0 ];
    /* The input buffer size is zero! */
    testHeaders.bufferLen = 0U;
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         0 /* rangeStart */,
                                         10 /* rangeEnd */ );
    TEST_ASSERT_EQUAL( HTTPInsufficientMemory, retCode );

    /* Length of headers > length of buffer.*/
    tearDown();
    testHeaders.pBuffer = &testBuffer[ 0 ];
    /* The input buffer size is zero! */
    testHeaders.headersLen = testHeaders.bufferLen + 1;
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         0 /* rangeStart */,
                                         10 /* rangeEnd */ );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );

    /* Test incorrect combinations of rangeStart and rangeEnd. */

    /* rangeStart > rangeEnd */
    tearDown();
    testHeaders.pBuffer = &testBuffer[ 0 ];
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         10 /* rangeStart */,
                                         5 /* rangeEnd */ );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );

    /* rangeStart == INT32_MIN */
    tearDown();
    testHeaders.pBuffer = &testBuffer[ 0 ];
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         INT32_MIN /* rangeStart */,
                                         HTTP_RANGE_REQUEST_END_OF_FILE /* rangeEnd */ );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );

    /* rangeStart is negative but rangeStart is non-End of File. */
    tearDown();
    testHeaders.pBuffer = &testBuffer[ 0 ];
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         -10 /* rangeStart */,
                                         HTTP_RANGE_REQUEST_END_OF_FILE + 1 /* rangeEnd */ );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );
    tearDown();
    testHeaders.pBuffer = &testBuffer[ 0 ];
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         -50 /* rangeStart */,
                                         -10 /* rangeEnd */ );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );
}

/**
 * @brief Test Insufficient memory failure when the buffer has one less byte than required.
 */
void test_Http_AddRangeHeader_Insufficient_Memory( void )
{
    setupBuffersWithPreexistingHeader( &testHeaders,
                                       testBuffer,
                                       sizeof( testBuffer ),
                                       &expectedHeaders,
                                       PREEXISTING_HEADER_DATA );
    size_t preHeadersLen = testHeaders.headersLen;
    testRangeStart = 5;
    testRangeEnd = 10;

    /* Update the expected header with the complete the range request header
     * to determine the total required size of the buffer. */
    addRangeToExpectedHeaders( &expectedHeaders,
                               "5-10" /*expected range*/,
                               1U );

    /* Change the input headers buffer size to be one byte short of the required
     * size to add Range Request header. */
    testHeaders.bufferLen = expectedHeaders.dataLen - 1;

    /* As the call to the API function is expected to fail, we need to store a
     * local copy of the input headers buffer to verify that the data has not changed
     * after the API call returns. Thus, overwrite the expected headers buffer with the
     * copy of the complete input headers buffer to use for verification later. */
    TEST_ASSERT_GREATER_OR_EQUAL( testHeaders.bufferLen, sizeof( expectedHeaders.buffer ) );
    memcpy( expectedHeaders.buffer, testHeaders.pBuffer, testHeaders.bufferLen );

    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         testRangeStart,
                                         testRangeEnd );
    TEST_ASSERT_EQUAL( HTTPInsufficientMemory, retCode );
    /* Verify the headers input parameter is unaltered. */
    TEST_ASSERT_EQUAL( preHeadersLen, testHeaders.headersLen );
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen - 1, testHeaders.bufferLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              testHeaders.pBuffer,
                              testHeaders.bufferLen );
}

/**
 * @brief Test addition of range header in a buffer not containing any header.
 */
void test_Http_AddRangeHeader_Without_Trailing_Terminator( void )
{
    /* Headers buffer does not contain data with trailing "\r\n\r\n". */

    /* Range specification of the form [rangeStart, rangeEnd]. */
    /* Test with 0 as the range values */
    setupBuffersWithPreexistingHeader( &testHeaders, testBuffer,
                                       sizeof( testBuffer ),
                                       &expectedHeaders,
                                       PREEXISTING_REQUEST_LINE );
    testRangeStart = 0;
    testRangeEnd = 0;
    addRangeToExpectedHeaders( &expectedHeaders,
                               "0-0" /*expected range*/,
                               0U );
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         testRangeStart,
                                         testRangeEnd );
    TEST_ASSERT_EQUAL( HTTPSuccess, retCode );
    /* Verify the the Range Request header data. */
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, testHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              testHeaders.pBuffer,
                              testHeaders.bufferLen );
    /* Verify that the bufferLen data was not tampered with. */
    TEST_ASSERT_EQUAL( sizeof( testBuffer ), testHeaders.bufferLen );
}

/**
 * @brief Test for Range specification of the form [rangeStart, rangeEnd].
 */
void test_Http_AddRangeHeader_RangeType_File_SubRange( void )
{
    /* Headers buffer contains header data ending with "\r\n\r\n". */

    /* Test with 0 as the range values */
    setupBuffersWithPreexistingHeader( &testHeaders, testBuffer,
                                       sizeof( testBuffer ),
                                       &expectedHeaders,
                                       PREEXISTING_HEADER_DATA );
    testRangeStart = 0;
    testRangeEnd = 0;
    addRangeToExpectedHeaders( &expectedHeaders,
                               "0-0" /*expected range*/,
                               1U );
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         testRangeStart,
                                         testRangeEnd );
    TEST_ASSERT_EQUAL( HTTPSuccess, retCode );
    /* Verify the the Range Request header data. */
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, testHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              testHeaders.pBuffer,
                              testHeaders.bufferLen );
    /* Verify that the bufferLen data was not tampered with. */
    TEST_ASSERT_EQUAL( sizeof( testBuffer ), testHeaders.bufferLen );

    tearDown();
    setupBuffersWithPreexistingHeader( &testHeaders, testBuffer,
                                       sizeof( testBuffer ),
                                       &expectedHeaders,
                                       PREEXISTING_HEADER_DATA );
    testRangeStart = 10;
    testRangeEnd = 100;
    addRangeToExpectedHeaders( &expectedHeaders,
                               "10-100" /*expected range*/,
                               1U );
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         testRangeStart,
                                         testRangeEnd );
    TEST_ASSERT_EQUAL( HTTPSuccess, retCode );
    /* Verify the the Range Request header data. */
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, testHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              testHeaders.pBuffer,
                              testHeaders.bufferLen );
    /* Verify that the bufferLen data was not tampered with. */
    TEST_ASSERT_EQUAL( sizeof( testBuffer ), testHeaders.bufferLen );
}

/**
 * @brief Test for adding request header for the [0, eof) range.
 */
void test_Http_AddRangeHeader_RangeType_Entire_File( void )
{
    setupBuffersWithPreexistingHeader( &testHeaders, testBuffer,
                                       sizeof( testBuffer ),
                                       &expectedHeaders,
                                       PREEXISTING_HEADER_DATA );
    testRangeStart = 0;
    testRangeEnd = HTTP_RANGE_REQUEST_END_OF_FILE;
    addRangeToExpectedHeaders( &expectedHeaders,
                               "0-" /*expected range*/,
                               1U );
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         testRangeStart,
                                         testRangeEnd );
    TEST_ASSERT_EQUAL( HTTPSuccess, retCode );
    /* Verify the the Range Request header data. */
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, testHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              testHeaders.pBuffer,
                              testHeaders.bufferLen );
    /* Verify that the bufferLen data was not tampered with. */
    TEST_ASSERT_EQUAL( sizeof( testBuffer ), testHeaders.bufferLen );
}

/**
 * @brief Test for Range specification of the form [rangeStart, eof).
 */
void test_Http_AddRangeHeader_RangeType_All_Bytes_From_RangeStart( void )
{
    /* Range specification of the form [rangeStart,)
     * i.e. for all bytes >= rangeStart. */
    tearDown();
    setupBuffersWithPreexistingHeader( &testHeaders, testBuffer,
                                       sizeof( testBuffer ),
                                       &expectedHeaders,
                                       PREEXISTING_HEADER_DATA );
    testRangeStart = 100;
    testRangeEnd = HTTP_RANGE_REQUEST_END_OF_FILE;
    addRangeToExpectedHeaders( &expectedHeaders,
                               "100-" /*expected range*/,
                               1U );
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         testRangeStart,
                                         testRangeEnd );
    TEST_ASSERT_EQUAL( HTTPSuccess, retCode );
    /* Verify the the Range Request header data. */
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, testHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              testHeaders.pBuffer,
                              testHeaders.bufferLen );
    /* Verify that the bufferLen data was not tampered with. */
    TEST_ASSERT_EQUAL( sizeof( testBuffer ), testHeaders.bufferLen );
}

/**
 * @brief Test for adding range request for the last N bytes.
 */
void test_Http_AddRangeHeader_RangeType_LastNBytes( void )
{
    /* Range specification for the last N bytes. */
    setupBuffersWithPreexistingHeader( &testHeaders, testBuffer,
                                       sizeof( testBuffer ),
                                       &expectedHeaders,
                                       PREEXISTING_HEADER_DATA );
    testRangeStart = -50;
    testRangeEnd = HTTP_RANGE_REQUEST_END_OF_FILE;
    addRangeToExpectedHeaders( &expectedHeaders,
                               "-50" /*expected range*/,
                               1U );
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         testRangeStart,
                                         testRangeEnd );
    TEST_ASSERT_EQUAL( HTTPSuccess, retCode );
    /* Verify the the Range Request header data. */
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, testHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              testHeaders.pBuffer,
                              testHeaders.bufferLen );
    /* Verify that the bufferLen data was not tampered with. */
    TEST_ASSERT_EQUAL( sizeof( testBuffer ), testHeaders.bufferLen );
}

/**
 * @brief Test addition of range request header with large integers.
 */
void test_Http_AddRangeHeader_With_Max_INT32_Range_Values( void )
{
    /* Test with LARGE range values. */
    setupBuffersWithPreexistingHeader( &testHeaders, testBuffer,
                                       sizeof( testBuffer ),
                                       &expectedHeaders,
                                       PREEXISTING_HEADER_DATA );
    testRangeStart = INT32_MAX;
    testRangeEnd = INT32_MAX;
    addRangeToExpectedHeaders( &expectedHeaders,
                               "2147483647-2147483647" /*expected range*/,
                               1U );
    retCode = HTTPClient_AddRangeHeader( &testHeaders,
                                         testRangeStart,
                                         testRangeEnd );
    TEST_ASSERT_EQUAL( HTTPSuccess, retCode );
    /* Verify the the Range Request header data. */
    TEST_ASSERT_EQUAL( expectedHeaders.dataLen, testHeaders.headersLen );
    TEST_ASSERT_EQUAL_MEMORY( expectedHeaders.buffer,
                              testHeaders.pBuffer,
                              testHeaders.bufferLen );
    /* Verify that the bufferLen data was not tampered with. */
    TEST_ASSERT_EQUAL( sizeof( testBuffer ), testHeaders.bufferLen );
}

/* ============== Testing HTTPClient_ReadHeader ================== */

/**
 * @brief Test with invalid parameter inputs.
 */
void test_Http_ReadHeader_Invalid_Params( void )
{
    /* Response parameter is NULL. */
    retCode = HTTPClient_ReadHeader( NULL,
                                     HEADER_INVALID_PARAMS,
                                     HEADER_INVALID_PARAMS_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );

    /* Underlying buffer is NULL in the response parameter. */
    tearDown();
    testResponse.pBuffer = NULL;
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_INVALID_PARAMS,
                                     HEADER_INVALID_PARAMS_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );

    /* Response buffer size is zero. */
    tearDown();
    testResponse.bufferLen = 0;
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_INVALID_PARAMS,
                                     HEADER_INVALID_PARAMS_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );

    /* Header field name is NULL. */
    tearDown();
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     NULL,
                                     HEADER_INVALID_PARAMS_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );

    /* Header field length is 0. */
    tearDown();
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_INVALID_PARAMS,
                                     0U,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );

    /* Invalid output parameters. */
    tearDown();
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_INVALID_PARAMS,
                                     HEADER_INVALID_PARAMS_LEN,
                                     NULL,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );
    tearDown();
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_INVALID_PARAMS,
                                     HEADER_INVALID_PARAMS_LEN,
                                     &pValueLoc,
                                     NULL );
    TEST_ASSERT_EQUAL( HTTPInvalidParameter, retCode );
}

/**
 * @brief Test when requested header is not present in response.
 */
void test_Http_ReadHeader_Header_Not_In_Response( void )
{
    /* Add expectations for llhttp dependencies. */
    llhttp_settings_init_ExpectAnyArgs();
    llhttp_init_ExpectAnyArgs();

    /* Configure the llhttp_execute mock. */
    invokeHeaderFieldCallback = 1U;
    invokeHeaderValueCallback = 1U;
    pFieldLocToReturn = &pTestResponse[ headerFieldInRespLoc ];
    fieldLenToReturn = headerFieldInRespLen;
    pValueLocToReturn = &pTestResponse[ headerValInRespLoc ];
    valueLenToReturn = headerValInRespLen;
    expectedValCbRetVal = LLHTTP_CONTINUE_PARSING;
    invokeHeaderCompleteCallback = 1U;
    parserErrNo = HPE_OK;
    llhttp_execute_ExpectAnyArgsAndReturn( HPE_OK );

    /* Call the function under test. */
    testResponse.bufferLen = strlen( pTestResponse );
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_NOT_IN_BUFFER,
                                     HEADER_NOT_IN_BUFFER_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPHeaderNotFound, retCode );

    /* Repeat the test above but with fieldLenToReturn == HEADER_NOT_IN_BUFFER_LEN.
     * Doing this allows us to take the branch where the actual contents
     * of the fields are compared rather than just the length. */
    setUp();
    /* Add expectations for llhttp dependencies. */
    llhttp_settings_init_ExpectAnyArgs();
    llhttp_init_ExpectAnyArgs();
    /* Ensure that the header field does NOT match what we're searching. */
    TEST_ASSERT_EQUAL( otherHeaderFieldInRespLen, HEADER_NOT_IN_BUFFER_LEN );
    TEST_ASSERT_TRUE( memcmp( &pTestResponse[ otherHeaderFieldInRespLoc ],
                              HEADER_NOT_IN_BUFFER,
                              HEADER_NOT_IN_BUFFER_LEN ) != 0 );
    /* Configure the llhttp_execute mock. */
    invokeHeaderFieldCallback = 1U;
    invokeHeaderValueCallback = 1U;
    pFieldLocToReturn = &pTestResponse[ otherHeaderFieldInRespLoc ];
    fieldLenToReturn = otherHeaderFieldInRespLen;
    pValueLocToReturn = &pTestResponse[ headerValInRespLoc ];
    valueLenToReturn = headerValInRespLen;
    expectedValCbRetVal = LLHTTP_CONTINUE_PARSING;
    invokeHeaderCompleteCallback = 1U;
    parserErrNo = HPE_OK;
    llhttp_execute_ExpectAnyArgsAndReturn( HPE_OK );

    /* Call the function under test. */
    testResponse.bufferLen = strlen( pTestResponse );
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_NOT_IN_BUFFER,
                                     HEADER_NOT_IN_BUFFER_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPHeaderNotFound, retCode );
}

/**
 * @brief Test with an invalid HTTP response containing only the field name the
 * requested header.
 */
void test_Http_ReadHeader_Invalid_Response_Only_Header_Field_Found()
{
    /* Test when invalid response only contains the header field for the requested header. */
    const char * pResponseWithoutValue = "HTTP/1.1 200 OK\r\n"
                                         "test-header0: test-value0\r\n"
                                         "test-header1:";

    /* Add expectations for llhttp init dependencies. */
    llhttp_settings_init_ExpectAnyArgs();
    llhttp_init_ExpectAnyArgs();

    /* Configure the llhttp_execute mock. */
    pExpectedBuffer = pResponseWithoutValue;
    expectedBufferSize = strlen( pResponseWithoutValue );
    invokeHeaderFieldCallback = 1U;
    pFieldLocToReturn = &pTestResponse[ headerFieldInRespLoc ];
    fieldLenToReturn = headerFieldInRespLen;
    llhttp_execute_ExpectAnyArgsAndReturn( HPE_OK );

    /* Call the function under test. */
    testResponse.pBuffer = ( uint8_t * ) &pResponseWithoutValue[ 0 ];
    testResponse.bufferLen = strlen( pResponseWithoutValue );
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_IN_BUFFER,
                                     HEADER_IN_BUFFER_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPInvalidResponse, retCode );
}


/**
 * @brief Test happy path with zero-initialized requestHeaders and requestInfo.
 * Use characters in the header with ASCII values higher than 'z'.
 */
void test_caseInsensitiveStringCmp()
{
    /* Add expectations for llhttp dependencies. */
    llhttp_settings_init_ExpectAnyArgs();
    llhttp_init_ExpectAnyArgs();

    /* Configure the llhttp_execute mock. */
    invokeHeaderFieldCallback = 1U;
    invokeHeaderValueCallback = 1U;
    pFieldLocToReturn = &pTestResponseSpecialCharacter[ headerFieldInRespLoc ];
    fieldLenToReturn = headerFieldInRespLen;
    pValueLocToReturn = &pTestResponseSpecialCharacter[ headerValInRespLoc ];
    valueLenToReturn = headerValInRespLen;
    expectedValCbRetVal = LLHTTP_CONTINUE_PARSING;
    invokeHeaderCompleteCallback = 1U;
    parserErrNo = HPE_OK;
    llhttp_execute_ExpectAnyArgsAndReturn( HPE_OK );

    /* Call the function under test. */
    testResponse.bufferLen = strlen( pTestResponseSpecialCharacter );
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_NOT_IN_BUFFER,
                                     HEADER_NOT_IN_BUFFER_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPHeaderNotFound, retCode );

    /* Repeat the test above but with fieldLenToReturn == HEADER_NOT_IN_BUFFER_LEN.
     * Doing this allows us to take the branch where the actual contents
     * of the fields are compared rather than just the length. */
    setUp();
    /* Add expectations for llhttp dependencies. */
    llhttp_settings_init_ExpectAnyArgs();
    llhttp_init_ExpectAnyArgs();
    /* Ensure that the header field does NOT match what we're searching. */
    TEST_ASSERT_EQUAL( otherHeaderFieldInRespLen, HEADER_NOT_IN_BUFFER_LEN );
    TEST_ASSERT_TRUE( memcmp( &pTestResponseSpecialCharacter[ otherHeaderFieldInRespLoc ],
                              HEADER_WITH_SPECIAL_CHARACTERS,
                              HEADER_WITH_SPECIAL_CHARACTERS_LEN ) != 0 );
    /* Configure the llhttp_execute mock. */
    invokeHeaderFieldCallback = 1U;
    invokeHeaderValueCallback = 1U;
    pFieldLocToReturn = &pTestResponseSpecialCharacter[ otherHeaderFieldInRespLoc ];
    fieldLenToReturn = otherHeaderFieldInRespLen;
    pValueLocToReturn = &pTestResponseSpecialCharacter[ headerValInRespLoc ];
    valueLenToReturn = headerValInRespLen;
    expectedValCbRetVal = LLHTTP_CONTINUE_PARSING;
    invokeHeaderCompleteCallback = 1U;
    parserErrNo = HPE_OK;
    llhttp_execute_ExpectAnyArgsAndReturn( HPE_OK );

    /* Call the function under test. */
    testResponse.bufferLen = strlen( pTestResponseSpecialCharacter );
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_WITH_SPECIAL_CHARACTERS,
                                     HEADER_WITH_SPECIAL_CHARACTERS_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPHeaderNotFound, retCode );
}

/**
 * @brief Test with an invalid HTTP response that does not contain terminating
 * characters ("\r\n\r\n") that represent the end of headers in the response.
 */
void test_Http_ReadHeader_Invalid_Response_No_Headers_Complete_Ending()
{
    /* Test response that does not contain requested header,
     * is invalid as it doesn't end with "\r\n\r\n". */
    const char * pResponseWithoutHeaders = "HTTP/1.1 200 OK\r\n"
                                           "test-header0:test-value0";

    tearDown();

    /* Add expectations for llhttp init dependencies. */
    llhttp_settings_init_ExpectAnyArgs();
    llhttp_init_ExpectAnyArgs();

    /* Configure the llhttp_execute mock. */
    pExpectedBuffer = &pResponseWithoutHeaders[ 0 ];
    expectedBufferSize = strlen( pResponseWithoutHeaders );
    /* Use -1 for an unknown error. */
    parserErrNo = -1;
    llhttp_execute_ExpectAnyArgsAndReturn( -1 );
    /* Call the function under test. */
    testResponse.pBuffer = ( uint8_t * ) &pResponseWithoutHeaders[ 0 ];
    testResponse.bufferLen = strlen( pResponseWithoutHeaders );
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_NOT_IN_BUFFER,
                                     HEADER_NOT_IN_BUFFER_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPInvalidResponse, retCode );
}

/**
 * @brief Test when the header is present in response but llhttp_execute()
 * does not set the expected errno value (of "HPE_USER")
 * due to an internal error.
 */
void test_Http_ReadHeader_With_HttpParser_Internal_Error()
{
    /* Add expectations for llhttp init dependencies. */
    llhttp_settings_init_ExpectAnyArgs();
    llhttp_init_ExpectAnyArgs();

    /* Configure the llhttp_execute mock. */
    invokeHeaderFieldCallback = 1U;
    invokeHeaderValueCallback = 1U;
    pFieldLocToReturn = &pTestResponse[ headerFieldInRespLoc ];
    fieldLenToReturn = headerFieldInRespLen;
    pValueLocToReturn = &pTestResponse[ headerValInRespLoc ];
    valueLenToReturn = headerValInRespLen;
    expectedValCbRetVal = LLHTTP_STOP_PARSING;
    parserErrNo = HPE_CB_CHUNK_COMPLETE;
    llhttp_execute_ExpectAnyArgsAndReturn( HPE_CB_CHUNK_COMPLETE );

    /* Call the function under test. */
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_IN_BUFFER,
                                     HEADER_IN_BUFFER_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPParserInternalError, retCode );
}

/**
 * @brief Test when requested header is present in the HTTP response.
 */
void test_Http_ReadHeader_Happy_Path()
{
    /* Add expectations for llhttp init dependencies. */
    llhttp_settings_init_ExpectAnyArgs();
    llhttp_init_ExpectAnyArgs();

    /* Configure the llhttp_execute mock. */
    expectedValCbRetVal = LLHTTP_STOP_PARSING;
    pFieldLocToReturn = &pTestResponse[ headerFieldInRespLoc ];
    fieldLenToReturn = headerFieldInRespLen;
    pValueLocToReturn = &pTestResponse[ headerValInRespLoc ];
    valueLenToReturn = headerValInRespLen;
    invokeHeaderFieldCallback = 1U;
    invokeHeaderValueCallback = 1U;
    /* Use HPE_USER to indicate the header value callback returns an error. */
    parserErrNo = HPE_USER;
    llhttp_execute_ExpectAnyArgsAndReturn( HPE_USER );

    /* Call the function under test. */
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_IN_BUFFER,
                                     HEADER_IN_BUFFER_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPSuccess, retCode );
    TEST_ASSERT_EQUAL( &pTestResponse[ headerValInRespLoc ], pValueLoc );
    TEST_ASSERT_EQUAL( headerValInRespLen, valueLen );
}

/**
 * @brief Test the case when the header is empty. Empty headers are not
 * invalid according to RFC 2616.
 */
void test_Http_ReadHeader_EmptyHeaderValue()
{
    /* Add expectations for llhttp init dependencies. */
    llhttp_settings_init_ExpectAnyArgs();
    llhttp_init_ExpectAnyArgs();

    /* Configure the llhttp_execute mock. */
    expectedValCbRetVal = LLHTTP_STOP_PARSING;
    pFieldLocToReturn = &pTestResponseEmptyValue[ headerFieldInRespLoc ];
    fieldLenToReturn = headerFieldInRespLen;
    /* Add two characters past the empty value to point to the next field. */
    pValueLocToReturn = &pTestResponseEmptyValue[ headerValInRespLoc + HTTP_HEADER_LINE_SEPARATOR_LEN ];
    /* llhttp will pass in a value of zero for an empty value. */
    valueLenToReturn = 0U;
    invokeHeaderFieldCallback = 1U;
    invokeHeaderValueCallback = 1U;
    /* Use HPE_USER to indicate the header value callback returns an error. */
    parserErrNo = HPE_USER;
    llhttp_execute_ExpectAnyArgsAndReturn( HPE_USER );

    /* Call the function under test. */
    retCode = HTTPClient_ReadHeader( &testResponse,
                                     HEADER_IN_BUFFER,
                                     HEADER_IN_BUFFER_LEN,
                                     &pValueLoc,
                                     &valueLen );
    TEST_ASSERT_EQUAL( HTTPSuccess, retCode );
    TEST_ASSERT_EQUAL( NULL, pValueLoc );
    TEST_ASSERT_EQUAL( 0U, valueLen );
}

/**
 * @brief Test HTTPClient_strerror returns correct strings.
 */
void test_HTTPClient_strerror( void )
{
    HTTPStatus_t status;
    const char * str = NULL;

    status = HTTPSuccess;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPSuccess", str );

    status = HTTPInvalidParameter;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPInvalidParameter", str );

    status = HTTPNetworkError;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPNetworkError", str );

    status = HTTPPartialResponse;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPPartialResponse", str );

    status = HTTPNoResponse;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPNoResponse", str );

    status = HTTPInsufficientMemory;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPInsufficientMemory", str );

    status = HTTPSecurityAlertExtraneousResponseData;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPSecurityAlertExtraneousResponseData", str );

    status = HTTPSecurityAlertInvalidChunkHeader;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPSecurityAlertInvalidChunkHeader", str );

    status = HTTPSecurityAlertInvalidProtocolVersion;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPSecurityAlertInvalidProtocolVersion", str );

    status = HTTPSecurityAlertInvalidStatusCode;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPSecurityAlertInvalidStatusCode", str );

    status = HTTPSecurityAlertInvalidCharacter;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPSecurityAlertInvalidCharacter", str );

    status = HTTPSecurityAlertInvalidContentLength;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPSecurityAlertInvalidContentLength", str );

    status = HTTPParserPaused;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPParserPaused", str );

    status = HTTPParserInternalError;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPParserInternalError", str );

    status = HTTPHeaderNotFound;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPHeaderNotFound", str );

    status = HTTPInvalidResponse;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( "HTTPInvalidResponse", str );

    status = HTTPInvalidResponse + 1;
    str = HTTPClient_strerror( status );
    TEST_ASSERT_EQUAL_STRING( NULL, str );
}

/* ========================================================================== */
