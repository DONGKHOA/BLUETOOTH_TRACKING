/*
 * coreMQTT Agent v1.2.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

/* MQTT agent include. */
#include "core_mqtt_agent.h"

void harness()
{
    MQTTAgentContext_t * pMqttAgentContext;
    MQTTAgentMessageInterface_t * pMsgInterface;
    MQTTFixedBuffer_t * pNetworkBuffer;
    TransportInterface_t * pTransportInterface;
    MQTTGetCurrentTimeFunc_t getCurrentTimeMs;
    MQTTAgentIncomingPublishCallback_t incomingCallback;
    void * pIncomingPacketContext;

    pMqttAgentContext = malloc( sizeof( MQTTAgentContext_t ) );
    pMsgInterface = malloc( sizeof( MQTTAgentMessageInterface_t ) );
    pNetworkBuffer = malloc( sizeof( MQTTFixedBuffer_t ) );
    pTransportInterface = malloc( sizeof( TransportInterface_t ) );

    MQTTAgent_Init( pMqttAgentContext,
                    pMsgInterface,
                    pNetworkBuffer,
                    pTransportInterface,
                    getCurrentTimeMs,
                    incomingCallback,
                    pIncomingPacketContext );
}
