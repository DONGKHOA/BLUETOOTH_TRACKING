/*
 * AWS IoT Over-the-air Update v3.3.0
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
 * @file ota_os_freertos.h
 * @brief Function declarations for the example OTA OS Functional interface for
 * FreeRTOS.
 */

#ifndef _OTA_OS_FREERTOS_H_
#define _OTA_OS_FREERTOS_H_

/* Standard library include. */
#include <stdint.h>
#include <string.h>

#include "ota_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @ingroup ota_enum_types
 * @brief The OTA OS interface return status.
 */
typedef enum OtaOsStatus
{
    OtaOsSuccess = 0, /*!< @brief OTA OS interface success. */
    OtaOsEventQueueCreateFailed = 0x80U, /*!< @brief Failed to create the event
                                            queue. */
    OtaOsEventQueueSendFailed,    /*!< @brief Posting event message to the event
                                     queue failed. */
    OtaOsEventQueueReceiveFailed, /*!< @brief Failed to receive from the event
                                     queue. */
    OtaOsEventQueueDeleteFailed,  /*!< @brief Failed to delete the event queue.
                                   */
} OtaOsStatus_t;

typedef enum OtaEvent
{
    OtaAgentEventStart = 0,           /*!< @brief Start the OTA state machine */
    OtaAgentEventRequestJobDocument,  /*!< @brief Event for requesting job document. */
    OtaAgentEventReceivedJobDocument, /*!< @brief Event when job document is received. */
    OtaAgentEventCreateFile,          /*!< @brief Event to create a file. */
    OtaAgentEventRequestFileBlock,    /*!< @brief Event to request file blocks. */
    OtaAgentEventReceivedFileBlock,   /*!< @brief Event to trigger when file block is received. */
    OtaAgentEventCloseFile,           /*!< @brief Event to trigger closing file. */
	OtaAgentEventActivateImage,       /*!< @brief Event to activate the new image. */
    OtaAgentEventSuspend,             /*!< @brief Event to suspend ota task */
    OtaAgentEventResume,              /*!< @brief Event to resume suspended task */
    OtaAgentEventUserAbort,           /*!< @brief Event triggered by user to stop agent. */
    OtaAgentEventShutdown,            /*!< @brief Event to trigger ota shutdown */
    OtaAgentEventMax                  /*!< @brief Last event specifier */
} OtaEvent_t;

typedef struct OtaDataEvent
{
    uint8_t data[ OTA_DATA_BLOCK_SIZE * 2 ]; /*!< Buffer for storing event information. */
    size_t dataLength;                 /*!< Total space required for the event. */
    bool bufferUsed;                     /*!< Flag set when buffer is used otherwise cleared. */
} OtaDataEvent_t;

typedef struct OtaJobEventData
{
    uint8_t jobData[ JOB_DOC_SIZE ];
    size_t jobDataLength;
} OtaJobEventData_t;

typedef struct OtaEventMsg
{
    OtaDataEvent_t * dataEvent; /*!< Data Event message. */
    OtaJobEventData_t * jobEvent; /*!< Job Event message. */
    OtaEvent_t eventId;          /*!< Identifier for the event. */
} OtaEventMsg_t;

/**
 * @brief Application version structure.
 */
typedef struct
{
    /* MISRA Ref 19.2.1 [Unions] */
    /* More details at: https://github.com/aws/ota-for-aws-iot-embedded-sdk/blob/main/MISRA.md#rule-192 */
    /* coverity[misra_c_2012_rule_19_2_violation] */
    union
    {
        #if ( defined( __BYTE_ORDER__ ) && defined( __ORDER_LITTLE_ENDIAN__ ) && ( __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ ) ) || ( __little_endian__ == 1 ) || WIN32 || ( __BYTE_ORDER == __LITTLE_ENDIAN )
            struct version
            {
                uint16_t build; /*!< @brief Build of the firmware (Z in firmware version Z.Y.X). */
                uint8_t minor;  /*!< @brief Minor version number of the firmware (Y in firmware version Z.Y.X). */

                uint8_t major;  /*!< @brief Major version number of the firmware (X in firmware version Z.Y.X). */
            } x;                /*!< @brief Version number of the firmware. */
        #elif ( defined( __BYTE_ORDER__ ) && defined( __ORDER_BIG_ENDIAN__ ) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ ) || ( __big_endian__ == 1 ) || ( __BYTE_ORDER == __BIG_ENDIAN )
            struct version
            {
                uint8_t major;  /*!< @brief Major version number of the firmware (X in firmware version X.Y.Z). */
                uint8_t minor;  /*!< @brief Minor version number of the firmware (Y in firmware version X.Y.Z). */

                uint16_t build; /*!< @brief Build of the firmware (Z in firmware version X.Y.Z). */
            } x;                /*!< @brief Version number of the firmware. */
        #else /* if ( defined( __BYTE_ORDER__ ) && defined( __ORDER_LITTLE_ENDIAN__ ) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ ) || ( __little_endian__ == 1 ) || WIN32 || ( __BYTE_ORDER == __LITTLE_ENDIAN ) */
        #error "Unable to determine byte order!"
        #endif /* if ( defined( __BYTE_ORDER__ ) && defined( __ORDER_LITTLE_ENDIAN__ ) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ ) || ( __little_endian__ == 1 ) || WIN32 || ( __BYTE_ORDER == __LITTLE_ENDIAN ) */
        uint32_t unsignedVersion32;
        int32_t signedVersion32;
    } u; /*!< @brief Version based on configuration in big endian or little endian. */
} AppVersion32_t;

/**
 * @brief Initialize the OTA events.
 *
 * This function initializes the OTA events mechanism for freeRTOS platforms.
 *
 * @return               OtaOsStatus_t, OtaOsSuccess if success , other error code on failure.
 */
OtaOsStatus_t OtaInitEvent_FreeRTOS( void );

/**
 * @brief Sends an OTA event.
 *
 * This function sends an event to OTA library event handler on FreeRTOS platforms.
 *
 * @param[pEventCtx]     Pointer to the OTA event context.
 *
 * @param[pEventMsg]     Event to be sent to the OTA handler.
 *
 * @param[timeout]       The maximum amount of time (msec) the task should block.
 *
 * @return               OtaOsStatus_t, OtaOsSuccess if success , other error code on failure.
 */
OtaOsStatus_t OtaSendEvent_FreeRTOS( const void * pEventMsg );

/**
 * @brief Receive an OTA event.
 *
 * This function receives next event from the pending OTA events on FreeRTOS platforms.
 *
 * @param[pEventCtx]     Pointer to the OTA event context.
 *
 * @param[pEventMsg]     Pointer to store message.
 *
 * @param[timeout]       The maximum amount of time the task should block.
 *
 * @return               OtaOsStatus_t, OtaOsSuccess if success , other error code on failure.
 */
OtaOsStatus_t OtaReceiveEvent_FreeRTOS( void * pEventMsg );

/**
 * @brief Deinitialize the OTA Events mechanism.
 *
 * This function deinitialize the OTA events mechanism and frees any resources
 * used on FreeRTOS platforms.
 *
 * @param[pEventCtx]     Pointer to the OTA event context.
 *
 * @return               OtaOsStatus_t, OtaOsSuccess if success , other error code on failure.
 */
OtaOsStatus_t OtaDeinitEvent_FreeRTOS( void );

/**
 * @brief Allocate memory.
 *
 * This function allocates the requested memory and returns a pointer to it on FreeRTOS platforms.
 *
 * @param[size]        This is the size of the memory block, in bytes..
 *
 * @return             This function returns a pointer to the allocated memory, or NULL if
 *                     the request fails.
 */

void * Malloc_FreeRTOS( size_t size );

/**
 * @brief Free memory.
 *
 * This function deallocates the memory previously allocated by a call to allocation
 * function of type OtaMalloc_t on FreeRTOS platforms.
 *
 * @param[size]        This is the pointer to a memory block previously allocated with function
 *                     of type OtaMalloc_t. If a null pointer is passed as an argument, no action occurs.
 *
 * @return             None.
 */

void Free_FreeRTOS( void * ptr );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _OTA_OS_FREERTOS_H_ */
