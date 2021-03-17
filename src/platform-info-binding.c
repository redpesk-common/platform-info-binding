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
#include "platform-info-devices.h"

#ifndef PLATFORM_INFO_DIR
#define PLATFORM_INFO_DIR "/etc/platform-info"
#endif

#ifndef HOTPLUG_NS
#define HOTPLUG_NS "hotplug"
#endif

#ifndef SCRIPTS_PATH
    #define SCRIPTS_PATH "@SCRIPTS_PATH@"
#endif

static int parseDataScrpitToJson(char * data, json_object * dataJ) {
	const char * separators = "/\n";
	// Extract each token and fill into a json object
	char * dataToken = strtok( data, separators);
	while (dataToken != NULL) {
		json_object_array_add(dataJ, json_object_new_string(dataToken));
		dataToken = strtok(NULL, separators);
	}
	return 0;
}

static int specialAction(afb_req_t req, char * input_name, json_object *resultJ) {
	char script_name[256], path[256], cmd[256];
    const char *scriptPathEnv = NULL;
	int err;
	DIR *d;
	FILE *fp;
	struct dirent *dir;
    json_object *elemJ = NULL;

	// add .sh to the name
	strcpy(script_name, input_name);
	strcat(script_name, ".sh");

	scriptPathEnv = getenv("SCRIPTS_PATH");
	if (!scriptPathEnv){
        AFB_API_NOTICE(req->api, "Using default acript path : %s", SCRIPTS_PATH);
        scriptPathEnv = SCRIPTS_PATH;
    }
    else AFB_API_NOTICE(req->api, "Found env script path : %s", scriptPathEnv);
	
	d = opendir(scriptPathEnv);

	if (d) {
		while ((dir = readdir(d)) != NULL) {
			// Compare with name given
			if ( strcmp(dir->d_name, script_name) == 0 ) {
				// Create command line
				strcpy(cmd, "sh ");
				strcat(cmd, scriptPathEnv);
				strcat(cmd, "/");
				strcat(cmd, script_name);

				// Open Command in reading mod
				fp = popen(cmd, "r");
				if (fp == NULL) goto PopenErrorExists;

				while (fgets(path, sizeof(path), fp) != NULL) {
					elemJ = json_object_new_array();
					err = parseDataScrpitToJson(path, elemJ);
					if (err) goto ParseErrorExists;
					json_object_array_add(resultJ, elemJ);
				}
				pclose(fp);
				closedir(d);
				return 1;
			}
		}
	}

	goto OnErrorExit;

PopenErrorExists:
	pclose(fp);
	AFB_API_ERROR(req->api, "Failed to run scripts");
	goto OnErrorExit;

ParseErrorExists:
	pclose(fp);
	AFB_API_ERROR(req->api, "Failed to parse data");
	goto OnErrorExit;

OnErrorExit:
	closedir(d);
	return 0;
}

void afv_get(afb_req_t req) {
	pinfo_api_ctx_t *api_ctx = (pinfo_api_ctx_t*)afb_api_get_userdata(req->api);

	if(api_ctx) {
		json_object *resultJ = NULL;
		json_object *args = afb_req_json(req);

		switch (json_object_get_type(args)) {
			case json_type_null:
				resultJ = api_ctx->info;
				break;
			case json_type_string: {
				char *json_path = NULL;
				char *full_json_path = NULL;

				full_json_path = strdupa(json_object_get_string(args));
				resultJ = api_ctx->info;

				for(json_path = strtok(full_json_path, "."); json_path && *json_path; json_path = strtok(NULL, ".")) {
					if(! json_object_object_get_ex(resultJ, (const char *)json_path, &resultJ)) {
						resultJ = json_object_new_array();
						if ( specialAction(req, json_path, resultJ) ) break;
						afb_req_fail(req, "A key hasn't been found in JSON path.", json_path);
						return;
					}
					printf("Found!\n");
				}
				break;
			}
			default:
				afb_req_fail(req, "Type error", "Argument type is unknown, you must provide a string only");
				return;
		}

		afb_req_success(req, json_object_get(resultJ), NULL);
	} else {
		afb_req_fail(req,"failed","The API contains no context!");
	}
}

void afv_get_all_info(afb_req_t req) {
	pinfo_api_ctx_t * api_ctx = (pinfo_api_ctx_t * )afb_api_get_userdata(req->api);
	if(!api_ctx) goto NoContextError;

	json_object *resultJ = NULL;
	json_object *responsJ = NULL;
	json_object *elemJ = NULL;
	const char** all_info_ptr = all_info;
	resultJ = api_ctx->info;

	while(*all_info_ptr != 0) {
		printf("%s \n", *all_info_ptr);
		if (!json_object_object_get_ex(api_ctx->info, *all_info_ptr, &resultJ)) {
			resultJ = json_object_new_array();
			if ( ! specialAction(req, (char *) *all_info_ptr, resultJ) ) {
				AFB_API_WARNING(req->api, "The Key %s, hasn't been found!", *all_info_ptr);
				resultJ = json_object_new_object();
				resultJ= json_object_new_string("Unknown");
			}
		}
		wrap_json_pack(&elemJ, "{so}", *all_info_ptr, resultJ);

		if (!responsJ) responsJ = elemJ; 
		wrap_json_object_add(responsJ, elemJ);
   		++all_info_ptr;
	}

	afb_req_success(req, responsJ, NULL);
	return;

NoContextError:
	afb_req_fail(req,"failed","The API contains no context!");
	
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


void afv_unsubscribe(afb_req_t req) {
	afb_req_success(req, NULL, NULL);
}

void afv_subscribe(afb_req_t req) {
	afb_req_success(req, NULL, NULL);
}

static json_object * afv_static_info(afb_api_t api, const char * dir) {
	struct dirent* dir_ent = NULL;
	json_object* static_info = NULL;

	DIR* dir_handle = opendir(dir);
	if (! dir_handle) goto NoDirectoryError;

	static_info = json_object_new_object();

	while( (dir_ent = readdir(dir_handle)) != NULL) {
		if(dir_ent->d_type == DT_REG && dir_ent->d_name[0] != '.') {
			json_object * current_file = NULL; 
			size_t filepath_len = strlen(dir) + strlen(dir_ent->d_name) + 2;
			char *filepath = alloca(filepath_len);

			filepath = strncpy(filepath, dir, filepath_len);
			filepath = strncat(filepath, "/", filepath_len - strlen(filepath) - 1);
			filepath = strncat(filepath, dir_ent->d_name, filepath_len - strlen(filepath) - 1);

			AFB_API_DEBUG(api, "File in processing: %s", filepath);
			current_file = json_object_from_file(filepath);
#if (JSON_C_VERSION_MAJOR_VERSION == 0 && JSON_C_VERSION_MINOR_VERSION >= 13)
			const char *error = NULL;
			if( (error = json_util_get_last_err()) ) {
				AFB_NOTICE("This file '%s' isn't a JSON file or have error: %s. len: %ld", filepath, error, filepath_len);
				continue;
			}
#else
			if( current_file == NULL ) {
				AFB_API_NOTICE(api, "This file '%s' isn't a JSON file or have error.", filepath);
				continue;
			}
#endif
			wrap_json_object_add(static_info, current_file);
			AFB_API_DEBUG(api, "JSON loaded: %s", json_object_to_json_string_ext(current_file, JSON_C_TO_STRING_PRETTY));
		}
	}

	return static_info;

NoDirectoryError:
	AFB_ERROR("The directory %s does not exist.", dir);
	goto OnErrorExit;

OnErrorExit:
	return NULL;

}

int init(afb_api_t api) {

	pinfo_api_ctx_t *api_ctx = NULL;
	int ret = PINFO_OK;

	api_ctx = malloc(sizeof(*api_ctx));

	if(api_ctx) {
		api_ctx->info = afv_static_info(api, PLATFORM_INFO_DIR);
		AFB_API_DEBUG(api,"The API static data: %s", json_object_to_json_string_ext(api_ctx->info, JSON_C_TO_STRING_PRETTY));
		api_ctx->client_count = 0;
		afb_api_set_userdata(api, (void*)api_ctx);
	} else {
		AFB_API_WARNING(api,"Failed to load the static data");
		ret = PINFO_ERR;
	}

	return ret;
	
}
