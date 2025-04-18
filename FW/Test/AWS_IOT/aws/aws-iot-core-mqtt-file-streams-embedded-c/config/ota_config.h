/*
 * AWS IoT Device SDK for Embedded C 202103.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * @file ota_config.h
 * @brief OTA user configurable settings.
 */

#ifndef OTA_CONFIG_H_
#define OTA_CONFIG_H_

#include "sdkconfig.h"
#include "MQTTFileDownloader.h"
#include "MQTTFileDownloader_defaults.h"

#define OTA_DATA_BLOCK_SIZE ( mqttFileDownloader_CONFIG_BLOCK_SIZE )
#define JOB_DOC_SIZE        ( 2048U )

#define EXTRACT_ARGS( ... ) __VA_ARGS__
#define STRIP_PARENS( X ) X
#define REMOVE_PARENS( X ) STRIP_PARENS( EXTRACT_ARGS X )

/* Logging configurations */
#if CONFIG_AWS_OTA_LOG_ERROR || CONFIG_AWS_OTA_LOG_WARN || CONFIG_AWS_OTA_LOG_INFO || CONFIG_AWS_OTA_LOG_DEBUG

    /* Set logging level for the AWS_OTA to highest level,
     * so any defined logging level below is printed. */
    #ifdef LOG_LOCAL_LEVEL
        #undef LOG_LOCAL_LEVEL
    #endif
    #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
    #include "esp_log.h"

    /* Change LIBRARY_LOG_NAME to "AWS_OTA" if defined somewhere else. */
    #ifdef LIBRARY_LOG_NAME
        #undef LIBRARY_LOG_NAME
    #endif
    #define LIBRARY_LOG_NAME "AWS_OTA"

#endif

/* Undefine logging macros if they were defined somewhere else like another AWS/FreeRTOS library. */
#ifdef LogError
    #undef LogError
#endif

#ifdef LogWarn
    #undef LogWarn
#endif

#ifdef LogInfo
    #undef LogInfo
#endif

#ifdef LogDebug
    #undef LogDebug
#endif

/* Define logging macros based on configurations in sdkconfig.h. */
#if CONFIG_AWS_OTA_LOG_ERROR
    #define LogError( message, ... ) ESP_LOGE( LIBRARY_LOG_NAME, REMOVE_PARENS( message ), ##__VA_ARGS__ )
#else
    #define LogError( message, ... )
#endif

#if CONFIG_AWS_OTA_LOG_WARN
    #define LogWarn( message, ... ) ESP_LOGW( LIBRARY_LOG_NAME, REMOVE_PARENS( message ), ##__VA_ARGS__ )
#else
    #define LogWarn( message, ... )
#endif

#if CONFIG_AWS_OTA_LOG_INFO
    #define LogInfo( message, ... ) ESP_LOGI( LIBRARY_LOG_NAME, REMOVE_PARENS( message ), ##__VA_ARGS__ )
#else
    #define LogInfo( message, ... )
#endif

#if CONFIG_AWS_OTA_LOG_DEBUG
    #define LogDebug( message, ... ) ESP_LOGD( LIBRARY_LOG_NAME, REMOVE_PARENS( message ), ##__VA_ARGS__ )
#else
    #define LogDebug( message, ... )
#endif

/************ End of logging configuration ****************/

#endif /* OTA_CONFIG_H_ */
