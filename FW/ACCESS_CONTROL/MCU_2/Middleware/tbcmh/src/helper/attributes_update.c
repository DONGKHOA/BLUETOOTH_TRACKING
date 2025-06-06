// Copyright 2022 liangzhuzhi2020@gmail.com, https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file is called by tbc_mqtt_helper.c/.h.

#include <string.h>

#include "esp_err.h"

#include "tbc_utils.h"
#include "tbc_mqtt_helper_internal.h"

#include "attributes_update.h"

static const char *TAG = "ATTRIBUTES_UPDATE";

int tbcmh_attributes_update(tbcmh_handle_t client, const char *attributes,
                                         int qos /*= 1*/, int retain /*= 0*/)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(attributes, ESP_FAIL);

    // Take semaphore
    if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
         // TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
         return ESP_FAIL;
    }

    // send package...
    int msg_id = tbcm_clientattributes_publish(client->tbmqttclient, attributes, qos, retain);

    // Give semaphore
    xSemaphoreGiveRecursive(client->_lock);
    return msg_id;
}

int tbcmh_attributes_update_ex(tbcmh_handle_t client, tbcmh_value_t *object,
                              int qos/*= 1*/, int retain/*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(object, ESP_FAIL);

     // send package...
     char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
     int msg_id = tbcmh_attributes_update(client, pack, qos/*= 1*/, retain/*= 0*/);
     cJSON_free(pack); // free memory

     return msg_id;
}

