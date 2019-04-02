/*
 * Copyright (C) 2016-2019 "IoT.bzh"
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

#ifndef PLATFORM_INFO_DIR
#define PLATFORM_INFO_DIR "/etc/platform-info"
#endif

#ifndef HOTPLUG_NS
#define HOTPLUG_NS "hotplug"
#endif

void afv_get(afb_req_t req) {
	char *json_path = NULL, *full_json_path = NULL;
	json_object *platform_info = (json_object*) afb_api_get_userdata(req->api);
	json_object *args = afb_req_json(req), *result = NULL;

	switch (json_object_get_type(args)) {
		case json_type_null:
			result = platform_info;
			break;
		case json_type_string:
			full_json_path = strdupa(json_object_get_string(args));
			result = platform_info;
			for(json_path = strtok(full_json_path, "."); json_path && *json_path; json_path = strtok(NULL, ".")) {
				if(! json_object_object_get_ex(result, json_path, &result)) {
					afb_req_fail(req, "A key hasn't been found in JSON path.", json_path);
					return;
				}
			}
			break;
		default:
			afb_req_fail(req, "Type error", "Argument type is unknown, you must provide a string only");
			return;
	}

	afb_req_success(req, json_object_get(result), NULL);
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

// TODO RFOR: interface with inotify and udev
void afv_unsubscribe(afb_req_t req) {
	afb_req_success(req, NULL, NULL);
}

void afv_subscribe(afb_req_t req) {
	afb_req_success(req, NULL, NULL);
}

int init(afb_api_t api) {
	struct dirent* dir_ent = NULL;

	DIR* dir_handle = opendir(PLATFORM_INFO_DIR);
	if (! dir_handle) {
		AFB_ERROR("The directory %s does not exist.", PLATFORM_INFO_DIR);
		return -1;
	}
	json_object *json_file = json_object_new_object(), *current_file = NULL;

	while( (dir_ent = readdir(dir_handle)) != NULL) {
		if(dir_ent->d_type == DT_REG && dir_ent->d_name[0] != '.') {
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
			wrap_json_object_add(json_file, current_file);
			AFB_DEBUG("JSON loaded: %s", json_object_get_string(json_file));
		}
	}

	// Initializing the platform_info binding object and associated it to
	// the api
	afb_api_set_userdata(api, (void*)json_file);

	return 0;
}
