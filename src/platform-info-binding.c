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
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "platform-info-binding.h"
#include "platform-info-devices.h"

#ifndef PLATFORM_INFO_DIR
#define PLATFORM_INFO_DIR "/etc/platform-info"
#endif

#ifndef HOTPLUG_NS
#define HOTPLUG_NS "hotplug"
#endif

#ifndef SCRIPTS_PATH
#define SCRIPTS_PATH "./var"
#endif

/**
 * Function: specialAction
 * -----------------------
 *
 * Used to run a script
 * in case the key specified in the request
 * isn't found in the json data file
 *
 * @param req	request of the client
 * @param input_name	string of a key in the json data
 * @param resultJ	the json data that is returned alongside an int
 *
 * @return 1 if everything is okay else 0
 */
static int specialAction(afb_req_t req, char *input_name, json_object *resultJ)
{
    char script_name[256], path[256], cmd[256];
    const char *scriptPathEnv = NULL;
    DIR *d;
    FILE *fp;
    struct dirent *dir;
    json_object *elemJ = NULL;

    // add .sh to the name
    strcpy(script_name, input_name);
    strcat(script_name, ".sh");

    scriptPathEnv = getenv("SCRIPTS_PATH");
    if (!scriptPathEnv) {
        AFB_REQ_NOTICE(req, "Using default script path : %s", SCRIPTS_PATH);
        scriptPathEnv = SCRIPTS_PATH;
    }
    else
        AFB_REQ_NOTICE(req, "Found env script path : %s/%s", scriptPathEnv, script_name);

    d = opendir(scriptPathEnv);
    if (!d)
        goto OpenDirError;

    while ((dir = readdir(d)) != NULL) {
        // Compare with name given
        if (strcmp(dir->d_name, script_name) == 0) {
            // Create command line
            strcpy(cmd, "sh ");
            strcat(cmd, scriptPathEnv);
            strcat(cmd, "/");
            strcat(cmd, script_name);

            // Open Command in reading mod
            fp = popen(cmd, "r");
            if (fp == NULL)
                goto PopenErrorExists;

            // Get script's result
            while (fgets(path, sizeof(path), fp) != NULL) {
                elemJ = json_tokener_parse(path);
                if (!elemJ)
                    goto ParseErrorExists;
                json_object_array_add(resultJ, elemJ);
            }

            pclose(fp);
            break;
        }
    }

    closedir(d);
    return 1;

OpenDirError:
    AFB_REQ_ERROR(req, "Failed to open directory");
    goto OnErrorExit;

PopenErrorExists:
    pclose(fp);
    AFB_REQ_ERROR(req, "Failed to run scripts");
    goto OnErrorExit;

ParseErrorExists:
    pclose(fp);
    AFB_REQ_ERROR(req, "Failed to parse output data from script %s", input_name);
    goto OnErrorExit;

OnErrorExit:
    closedir(d);
    return 0;
}

extern const char *info_verbS;

/**
 * Function: infoVerb
 * --------------------
 * a callback to the verb 'info'
 *
 * @param request		Request from the client
 * @param argc      Arguments count
 * @param argv      array of arguments
 *
 * @returns: Void
 */
void afv_info(afb_req_t req, unsigned argc, afb_data_t const argv[])
{
    enum json_tokener_error jerr;

    json_object *infoArgJ = json_tokener_parse_verbose(info_verbS, &jerr);
    if (infoArgJ == NULL || jerr != json_tokener_success) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST,
                             "failure while packing info() verb arguments");
        return;
    }
    afb_req_reply_json_c_hold(req, 0, infoArgJ);
    return;
}

/** Function:  afv_get
 * ----------------------
 * Callback for get verb.
 * It build a json data structure of the asked hardware specification and returns it
 *
 * @param request	Request from the client
 * @param argc      Arguments count
 * @param argv      Array of arguments
 *
 * @returns: nothing
 */
void afv_get(afb_req_t req, unsigned argc, afb_data_t const argv[])
{
    pinfo_api_ctx_t *api_ctx = (pinfo_api_ctx_t *)afb_api_get_userdata(afb_req_get_api(req));

    if (api_ctx) {
        afb_data_t arg;
        json_object *resultJ = NULL;
        if (afb_req_param_convert(req, 0, AFB_PREDEFINED_TYPE_JSON_C, &arg) < 0) {
            afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST,
                                 "failed to convert argument to JSON_C");
            return;
        }

        json_object *json_request = (json_object *)afb_data_ro_pointer(arg);
        if (!json_request) {
            afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST,
                                 "failed to get pointer to argument");
            return;
        }
        switch (json_object_get_type(json_request)) {
        case json_type_null:
            resultJ = api_ctx->info;
            break;
        case json_type_string: {
            char *json_path = NULL;
            char *full_json_path = NULL;

            full_json_path = strdupa(json_object_get_string(json_request));
            resultJ = api_ctx->info;

            for (json_path = strtok(full_json_path, "."); json_path && *json_path;
                 json_path = strtok(NULL, ".")) {
                if (!json_object_object_get_ex(resultJ, (const char *)json_path, &resultJ)) {
                    resultJ = json_object_new_array();
                    if (specialAction(req, json_path, resultJ))
                        break;
                    afb_req_reply_string_copy(req, AFB_ERRNO_INVALID_REQUEST, json_path, 0);
                    return;
                }
            }
            break;
        }
        default:
            afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST,
                                 "Argument type is unknown, you must provide a string only");
            return;
        }

        afb_req_reply_json_c_hold(req, 0, resultJ);
    }
    else {
        afb_req_reply_string(req, AFB_ERRNO_BAD_API_STATE, "The API contains no context!");
    }
}

/** Function:  afv_get_all_info
 * ----------------------
 * Callback for get_all_info verb.
 * It build a json data structure of the hardware specifications and returns it
 *
 * @param request	Request from the client
 * @param argc      Arguments count
 * @param argv      Array of arguments
 *
 * @returns: nothing
 */
void afv_get_all_info(afb_req_t req, unsigned argc, afb_data_t const argv[])
{
    pinfo_api_ctx_t *api_ctx = (pinfo_api_ctx_t *)afb_api_get_userdata(afb_req_get_api(req));
    if (!api_ctx)
        goto NoContextError;

    json_object *resultJ = NULL;
    json_object *responseJ = NULL;
    const char **all_info_ptr = all_info;

    while (*all_info_ptr) {
        if (!json_object_object_get_ex(api_ctx->info, *all_info_ptr, &resultJ)) {
            resultJ = json_object_new_array();
            if (!specialAction(req, (char *)*all_info_ptr, resultJ)) {
                AFB_REQ_WARNING(req, "The Key %s, hasn't been found!", *all_info_ptr);
                resultJ = json_object_new_object();
                resultJ = json_object_new_string("Unknown");
            }
        }
        else {
            resultJ = json_object_get(resultJ);
        }

        if (!responseJ) {
            rp_jsonc_pack(&responseJ, "{so}", *all_info_ptr, resultJ);
        }
        else {
            json_object_object_add(responseJ, *all_info_ptr, resultJ);
        }
        ++all_info_ptr;
    }

    afb_req_reply_json_c_hold(req, 0, responseJ);
    return;

NoContextError:
    afb_req_reply_string(req, AFB_ERRNO_BAD_API_STATE, "The API contains no context!");
}

/** Function:  afv_set
 * ----------------------
 * Callback for set verb.
 *
 * @param request	Request from the client
 * @param argc      Arguments count
 * @param argv      Array of arguments
 *
 * @returns: nothing
 */
void afv_set(afb_req_t req, unsigned argc, afb_data_t const argv[])
{
    json_object *platform_info = (json_object *)afb_req_get_userdata(req);
    afb_data_t arg;
    if (afb_req_param_convert(req, 0, AFB_PREDEFINED_TYPE_JSON_C, &arg) < 0) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST,
                             "failed to convert argument to JSON_C");
        return;
    }

    json_object *json_request = (json_object *)afb_data_ro_pointer(arg);
    if (!json_request) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST, "failed to get pointer to argument");
        return;
    }

#if (JSON_C_VERSION_MAJOR_VERSION == 0 && JSON_C_VERSION_MINOR_VERSION >= 13)
    if (json_object_object_add(platform_info, HOTPLUG_NS, args)) {
        afb_req_fail(req, "Addition fails", NULL);
        return;
    }
#else
    json_object_object_add(platform_info, HOTPLUG_NS, json_request);
#endif

    afb_req_reply(req, 0, 1, argv);
}

/** Function: check_subscribe_unsubscribe_arguments
 * Check if the event it's subscribing to is well created
 *
 * @param req 	The request of the client
 *
 * @return: True if everything is okay, else false
 */
static bool check_subscribe_unsubcribe_argument(afb_req_t req)
{
    afb_data_t result;
    if (afb_req_param_convert(req, 0, AFB_PREDEFINED_TYPE_JSON_C, &result) < 0) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST,
                             "failed to convert argument to JSON_C");
        return false;
    }
    json_object *json_request = (json_object *)afb_data_ro_pointer(result);
    if (!json_request) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST, "failed to get pointer to argument");
        return false;
    }

    json_object *event_json_object;
    if (!json_object_object_get_ex(json_request, "event", &event_json_object)) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST, "No 'event' value provided.");
        return false;
    }
    if (!json_object_is_type(event_json_object, json_type_string)) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST, "'event' must be a string");
        return false;
    }
    const char *event_name = json_object_get_string(event_json_object);
    if (!event_name || (strcmp(event_name, "monitor-devices") != 0)) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST, "Invalid event name");
        return false;
    }
    return true;
}

/** Function:  Unsubscribe
 * ----------------------
 * Callback for "unsubscribe" verb.
 * Unsubscribe a client to a dynamic event.
 *
 * @param request	Request from the client
 * @param argc      Arguments count
 * @param argv      Array of arguments
 *
 * @returns: nothing
 */
void afv_unsubscribe(afb_req_t req, unsigned argc, afb_data_t const argv[])
{
    if (!check_subscribe_unsubcribe_argument(req)) {
        return;
    }

    pinfo_client_ctx_t *client_ctx = NULL;
    afb_req_context_get(req, (void **)&client_ctx);
    if (!client_ctx) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST,
                             "No context available for the client.");
        return;
    }

    if (afb_req_unsubscribe(req, client_ctx->ev_devs_changed) < 0) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST, "Cannot unsubscribe");
    }
    else {
        // release the context
        afb_req_context_drop(req);
        afb_req_reply(req, 0, 0, NULL);
    }
}

/** Function:  Subscribe
 * --------------------
 * Callback for "subscribe" verb.
 * Subscribe a client to a dynamic event.
 * Create the asked event if not found in the event list.
 * Events are Udev's event
 *
 * @param request	Request from the client
 * @param argc      Arguments count
 * @param argv      Array of arguments
 *
 * @returns: nothing
 */
void afv_subscribe(afb_req_t req, unsigned argc, afb_data_t const argv[])
{
    if (!check_subscribe_unsubcribe_argument(req)) {
        return;
    }

    void *ptrval;
    afb_req_context_get(req, &ptrval);
    if (ptrval != NULL) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST, "The client already subscribed.");
    }

    if (pinfo_device_monitor(req) != PINFO_OK) {
        afb_req_reply_string(req, AFB_ERRNO_INVALID_REQUEST, "Unable to create new context");
    }

    afb_req_reply(req, 0, 0, NULL);
}

/**
 * Function: afv_static_info
 * -------------------------
 * Used to fetch the static data of the board when starting the binder
 *
 * @param api	the api to expose the return
 * @param dir	the directory of where to fetch hardware's data
 *
 * @return a Json object containing the data of the hardware that is exposed on the api
 */
static json_object *afv_static_info(afb_api_t api, const char *dir)
{
    struct dirent *dir_ent = NULL;
    json_object *static_info = NULL;

    DIR *dir_handle = opendir(dir);
    if (!dir_handle)
        goto NoDirectoryError;

    static_info = json_object_new_object();

    while ((dir_ent = readdir(dir_handle)) != NULL) {
        if (dir_ent->d_type == DT_REG && dir_ent->d_name[0] != '.') {
            json_object *current_file = NULL;
            size_t filepath_len = strlen(dir) + strlen(dir_ent->d_name) + 2;
            char *filepath = alloca(filepath_len);

            filepath = strncpy(filepath, dir, filepath_len);
            filepath = strncat(filepath, "/", filepath_len - strlen(filepath) - 1);
            filepath = strncat(filepath, dir_ent->d_name, filepath_len - strlen(filepath) - 1);

            AFB_API_DEBUG(api, "File in processing: %s", filepath);
            current_file = json_object_from_file(filepath);
#if (JSON_C_VERSION_MAJOR_VERSION == 0 && JSON_C_VERSION_MINOR_VERSION >= 13)
            const char *error = NULL;
            if ((error = json_util_get_last_err())) {
                AFB_NOTICE("This file '%s' isn't a JSON file or have error: %s. len: %ld", filepath,
                           error, filepath_len);
                continue;
            }
#else
            if (current_file == NULL) {
                AFB_API_NOTICE(api, "This file '%s' isn't a JSON file or have error.", filepath);
                continue;
            }
#endif
            rp_jsonc_object_add(static_info, current_file);
            AFB_API_DEBUG(api, "JSON loaded: %s",
                          json_object_to_json_string_ext(current_file, JSON_C_TO_STRING_PRETTY));
            json_object_put(current_file);
        }
    }
    closedir(dir_handle);

    return static_info;

NoDirectoryError:
    AFB_API_ERROR(api, "The directory %s does not exist.", dir);
    goto OnErrorExit;

OnErrorExit:
    return NULL;
}

static const struct afb_auth _afb_auths_platform_info[] = {
    {.type = afb_auth_Permission, .text = "urn:AGL:permission::platform:info:get"},
    {.type = afb_auth_Permission, .text = "urn:AGL:permission::platform:info:set"}};

/**
 * Binding CallBack
 * @param api the api that receive the CallBack
 * @param ctlid     identifier of the reason of the call (@see afb_ctlid)
 * @param ctlarg    data associated to the call
 * @param userdata  the userdata of the api (@see afb_api_get_userdata)
 */
int binding_ctl(afb_api_t api, afb_ctlid_t ctlid, afb_ctlarg_t ctlarg, void *userdata)
{
    switch (ctlid) {
    case afb_ctlid_Init:
        pinfo_api_ctx_t *api_ctx = NULL;
        int ret = PINFO_OK;
        api_ctx = malloc(sizeof(*api_ctx));
        if (api_ctx) {
            api_ctx->info = afv_static_info(api, PLATFORM_INFO_DIR);
            AFB_API_DEBUG(api, "The API static data: %s",
                          json_object_to_json_string_ext(api_ctx->info, JSON_C_TO_STRING_PRETTY));
            api_ctx->client_count = 0;
            afb_api_set_userdata(api, (void *)api_ctx);
        }
        else {
            AFB_API_WARNING(api, "Failed to load the static data");
            ret = PINFO_ERR;
        }
        return ret;
    default:
        break;
    }
    return 0;
}

// clang-format off
static const afb_verb_t _afb_verbs_platform_info[] = {
    {.verb = "info",
     .callback = afv_info,
     .auth = &_afb_auths_platform_info[0],
     .info = "Info regarding platform-info binding.",
     .session = AFB_SESSION_NONE
    },

    {.verb = "get",
     .callback = afv_get,
     .auth = &_afb_auths_platform_info[0],
     .info = "Get a platform data.",
     .session = AFB_SESSION_NONE
    },

    {.verb = "get_all_info",
     .callback = afv_get_all_info,
     .auth = &_afb_auths_platform_info[0],
     .info = "Get a set of platform data.",
     .session = AFB_SESSION_NONE
    },

    {.verb = "set",
     .callback = afv_set,
     .auth = &_afb_auths_platform_info[1],
     .info = "Set a platform data.",
     .session = AFB_SESSION_NONE},
    

    {.verb = "unsubscribe",
     .callback = afv_unsubscribe,
     .auth = NULL,
     .info = "Unsubscribe to changes (hotplug event, failures, ...)",
     .session = AFB_SESSION_NONE
    },

    {.verb = "subscribe",
     .callback = afv_subscribe,
     .auth = NULL,
     .info = "Subscribe to changes (hotplug event, failures, ...)",
     .session = AFB_SESSION_NONE
    },

    {.verb = NULL,
     .callback = NULL,
     .auth = NULL,
     .info = NULL,
     .session = 0
    }
};
// clang-format on

const struct afb_binding_v4 afbBindingExport = {
    .api = "platform-info",
    .specification = NULL,
    .info = "",
    .verbs = _afb_verbs_platform_info,
    .mainctl = binding_ctl,
};
