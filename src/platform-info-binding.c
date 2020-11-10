/*
 * Copyright (C) 2016-2020 "IoT.bzh"
 *
 * Author "Romain Forlot" <romain.forlot@iot.bzh>
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

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include <afb/afb-binding.h>
#include "platform-info-binding.h"
#include "platform-info-devices.h"

#ifndef PLATFORM_INFO_DIR
#define PLATFORM_INFO_DIR "/etc/platform-info"
#endif

#ifndef HOTPLUG_NS
#define HOTPLUG_NS "hotplug"
#endif

void afv_get(afb_req_t req) {
	pinfo_api_ctx_t *api_ctx = (pinfo_api_ctx_t*)afb_api_get_userdata(req->api);

	if(api_ctx) {
		json_object *result = NULL;
		json_object *args = afb_req_json(req);

		switch (json_object_get_type(args)) {
			case json_type_null:
				result = api_ctx->info;
				break;
			case json_type_string: {
				char *json_path = NULL;
				char *full_json_path = NULL;

				full_json_path = strdupa(json_object_get_string(args));
				result = api_ctx->info;

				for(json_path = strtok(full_json_path, ".");
					json_path && *json_path;
					json_path = strtok(NULL, ".")) {
					if(! json_object_object_get_ex(result, json_path, &result)) {
						afb_req_fail(req, "A key hasn't been found in JSON path.", json_path);
						return;
					}
				}
				break;
			}
			default:
				afb_req_fail(req, "Type error", "Argument type is unknown, you must provide a string only");
				return;
		}

		afb_req_success(req, json_object_get(result), NULL);
	} else {
		afb_req_fail(req,"failed","The API contains no context!");
	}
}

void afv_set(afb_req_t req) {
	json_object *platform_info = (json_object*) afb_api_get_userdata(req->api);
	json_object *args = afb_req_json(req);

#if (JSON_C_VERSION_MAJOR_VERSION == 0 && JSON_C_VERSION_MINOR_VERSION >= 13)
	if(json_object_object_add(platform_info, HOTPLUG_NS, args)) {
		afb_req_fail(req, "Addition fails", NULL);
		return;
	}
#else
	json_object_object_add(platform_info, HOTPLUG_NS, args);
#endif

	afb_req_success(req, NULL, NULL);
}

// TODO RFOR: interface with inotify
void afv_unsubscribe(afb_req_t req) {
	pinfo_client_ctx_t* client_ctx = NULL;
	client_ctx = (pinfo_client_ctx_t*)afb_req_context_get(req);

	if(client_ctx) {
		const char* event_str = afb_req_value(req,"event");

		if(event_str && strcmp("monitor-devices",event_str) == 0) {
			afb_req_unsubscribe(req,client_ctx->ev_devs_changed);
			afb_req_context_clear(req);
			afb_req_success(req,NULL, NULL);
		} else {
			afb_req_fail(req,"failed","No 'event' value provided.");
		}
	} else {
		afb_req_fail(req,"failed","No context available for the client.");
	}
}

void afv_subscribe(afb_req_t req) {
	const char* event_str = NULL;
	event_str = afb_req_value(req,"event");

	if(event_str && strcmp("monitor-devices",event_str) == 0) {
		if(!afb_req_context_get(req)) {
			if(pinfo_device_monitor(req) == PINFO_OK) {
				afb_req_success(req,NULL,NULL);
			} else {
				afb_req_fail(req,"failed","Unable to create new context");
			}
		} else {
			afb_req_fail(req,"failed","The client already subscribed.");
		}
	} else {
		afb_req_fail(req,"failed","Invalid event subscription");
	}
}

static json_object*
afv_static_info(const char* dir) {
	struct dirent* dir_ent = NULL;
	DIR* dir_handle = opendir(dir);
	json_object* static_info = NULL;

	if (! dir_handle) {
		AFB_ERROR("The directory %s does not exist.", PLATFORM_INFO_DIR);
		return NULL;
	}

	static_info = json_object_new_object();
	while( (dir_ent = readdir(dir_handle)) != NULL) {
		if(dir_ent->d_type == DT_REG && dir_ent->d_name[0] != '.') {
			json_object* current_file = NULL;
			size_t filepath_len = strlen(PLATFORM_INFO_DIR) + strlen(dir_ent->d_name) + 2;
			char *filepath = alloca(filepath_len);

			filepath = strncpy(filepath, PLATFORM_INFO_DIR, filepath_len);
			filepath = strncat(filepath, "/", filepath_len - strlen(filepath) - 1);
			filepath = strncat(filepath, dir_ent->d_name, filepath_len - strlen(filepath) - 1);

			AFB_DEBUG("File in processing: %s", filepath);
			current_file = json_object_from_file(filepath);
#if (JSON_C_VERSION_MAJOR_VERSION == 0 && JSON_C_VERSION_MINOR_VERSION >= 13)
			const char *error = NULL;
			if( (error = json_util_get_last_err()) ) {
				AFB_NOTICE("This file '%s' isn't a JSON file or have error: %s. len: %ld", filepath, error, filepath_len);
				continue;
			}
#else
			if( current_file == NULL ) {
				AFB_NOTICE("This file '%s' isn't a JSON file or have error.", filepath);
				continue;
			}
#endif
			wrap_json_object_add(static_info, current_file);
			AFB_DEBUG("JSON loaded: %s",
			json_object_to_json_string_ext(current_file,JSON_C_TO_STRING_PRETTY));
		}
	}

	return static_info;
}

int init(afb_api_t api) {
	// Initializing the platform_info binding object and associated it to
	// the api
	AFB_DEBUG("init() ...");
	pinfo_api_ctx_t *api_ctx = NULL;
	int ret = PINFO_OK;

	api_ctx = malloc(sizeof(*api_ctx));

	if(api_ctx) {
		AFB_DEBUG("init() ... OK");
		api_ctx->info = afv_static_info(PLATFORM_INFO_DIR);
		AFB_API_DEBUG(api,"The API static data: %s",
		json_object_to_json_string_ext(api_ctx->info, JSON_C_TO_STRING_PRETTY));
		api_ctx->client_count = 0;
		afb_api_set_userdata(api, (void*)api_ctx);
	} else {
		AFB_API_WARNING(api,"Failed to load the static data");
		ret = PINFO_ERR;
	}

	return ret;
}
