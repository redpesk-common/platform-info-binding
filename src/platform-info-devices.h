/*
 * Copyright (C) 2020 "IoT.bzh"
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLATFORM_INFO_DEVICE_H
#define PLATFORM_INFO_DEVICE_H

#include <afb/afb-binding.h>
#include "json-c/json.h"

#define PINFO_OK               (0)
#define PINFO_ERR              (-1)

struct udev;
struct udev_monitor;
struct pthread_t;

typedef struct {
    struct json_object* info;
    int client_count;
}pinfo_api_ctx_t;

typedef struct {
    struct udev *udev_ctx;
    struct udev_monitor *umon_hndl;
    struct json_object *filter;
    struct json_object *mask;
    pinfo_api_ctx_t *api_ctx;
    void(*umon_cb)(void* client_ctx, struct json_object* jdevice);
    pthread_t th;
    afb_event_t ev_devs_changed;
}pinfo_client_ctx_t;


int pinfo_device_monitor(afb_req_t req);

#endif
