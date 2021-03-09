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
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <libudev.h>
#include <unistd.h>
#include "platform-info-devices.h"

#define JSON_NEW_CONST_KEY \
    (JSON_C_OBJECT_KEY_IS_CONSTANT | JSON_C_OBJECT_ADD_KEY_IS_NEW)
#define UNUSED(x) (void)(x);

static void*        pinfo_device_client_new(void* req_ctx);
static void         pinfo_device_client_free(void* client_ctx);
static void*        pinfo_device_monitor_loop(pinfo_client_ctx_t* ctx);
static void         pinfo_device_monitor_detect(pinfo_client_ctx_t* ctx, struct json_object* jdevice);
static json_object* pinfo_device_udevice_to_jdevice(struct udev_device* udevice, struct json_object* jmask);
static void         pinfo_device_jdev_destructor(json_object* jdevice, void* udevice);
static int          pinfo_device_filter_monitoring(pinfo_client_ctx_t* ctx);
static void         pinfo_device_filter_scan(struct udev_enumerate* udev_enum, struct json_object* jfilter);
static json_object* pinfo_device_udevice_to_jlist(
                                    struct udev_device* udevice,
                                    struct udev_list_entry*(*udevice_elist)(struct udev_device*),
                                    const char*(*elist_val_getter)(struct udev_device*,const char*),
                                    const unsigned int jflags);
static json_object* pinfo_device_udevice_to_jlist_mask(
                                    struct udev_device* udevice,
                                    const char*(*udevice_val_getter)(struct udev_device*,const char*),
                                    json_object* jkeys,
                                    unsigned jcpy_flags);

static void
pinfo_device_monitor_detect(pinfo_client_ctx_t* ctx, struct json_object* jdevice)
{
	afb_event_push((afb_event_t)ctx->ev_devs_changed,jdevice);
}

int
pinfo_device_monitor(afb_req_t req) {
    int res = PINFO_ERR;

    if(afb_req_context(req,0,pinfo_device_client_new,pinfo_device_client_free,req)) {
        res = PINFO_OK;
    }

    return res;
}

static void
pinfo_device_client_free(void* client_ctx) {
	if(client_ctx) {
		pinfo_client_ctx_t* ctx = (pinfo_client_ctx_t*)client_ctx;
		if(ctx) {
			pthread_cancel(ctx->th);
			udev_monitor_unref(ctx->umon_hndl);
			udev_unref(ctx->udev_ctx);
			afb_event_unref((afb_event_t)ctx->ev_devs_changed);
			json_object_put(ctx->filter);
			json_object_put(ctx->mask);
			ctx->api_ctx->client_count--;
			AFB_DEBUG("Client context released, client count: %d",ctx->api_ctx->client_count);
		}
	}
}

static void*
pinfo_device_client_new(void* req_ctx) {
    pinfo_client_ctx_t* ctx = NULL;
	afb_req_t req = (afb_req_t)(req_ctx);

	if(req) {
        ctx = malloc(sizeof(*ctx));
        if(ctx) {
            ctx->udev_ctx =  udev_new();
            if(ctx->udev_ctx) {
                ctx->umon_hndl = udev_monitor_new_from_netlink(ctx->udev_ctx,"udev");
                ctx->filter = NULL;
                ctx->mask = NULL;
                ctx->umon_cb = NULL;

                if(ctx->umon_hndl) {
                    json_object* jval = NULL;
                    json_object* jargs = afb_req_json(req);
                    pthread_attr_t attr;

                    ctx->ev_devs_changed = afb_api_make_event(req->api,"device_changed");
                    if(afb_event_is_valid(ctx->ev_devs_changed)) {
                        if(afb_req_subscribe(req,ctx->ev_devs_changed) == PINFO_OK) {
                            ctx->umon_cb = (void(*)(void*,struct json_object*))pinfo_device_monitor_detect;
                            if(json_object_object_get_ex(jargs,"filter",&jval) &&
                                json_object_is_type(jval,json_type_object)) {
                                json_object_deep_copy(jval,&ctx->filter,NULL);
                            }

                            if(json_object_object_get_ex(jargs,"mask",&jval) &&
                                json_object_is_type(jval,json_type_object)) {
                                json_object_deep_copy(jval,&ctx->mask,NULL);
                            }

                            if(pinfo_device_filter_monitoring(ctx) == PINFO_OK) {
                                pthread_attr_init(&attr);
                                pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
                                if(pthread_create(&ctx->th,&attr,
                                (void*(*)(void*))&pinfo_device_monitor_loop,(void*)ctx) == PINFO_OK) {
                                    pinfo_api_ctx_t* api_ctx = NULL;
                                    api_ctx = (pinfo_api_ctx_t*)afb_api_get_userdata(req->api);
                                    AFB_REQ_DEBUG(req,"New client event-loop have been started ...");
                                    //New client (new session) get a pointer of API context
                                    //to access global(API) context
                                    if(api_ctx) {
                                        ctx->api_ctx = api_ctx;
                                        api_ctx->client_count++;
                                        AFB_REQ_INFO(req,"New client context allocated, client count: %d",
                                        api_ctx->client_count);
                                    }
                                } else {
                                    AFB_REQ_ERROR(req,"Failed to run the new session monitoring thread.");
                                    pinfo_device_client_free(ctx);
                                    ctx = NULL;
                                }
                                pthread_attr_destroy(&attr);
                            } else {
                                AFB_REQ_ERROR(req,"Failed to apply the device monitoring filters.");
                            }
                        } else {
                            AFB_REQ_ERROR(req,"New client event subscribing failed.");
                        }
                    } else {
                        AFB_REQ_ERROR(req,"Invalid AFB event generated for new client.");
                    }
                } else {
                    AFB_REQ_ERROR(req,
                    "Failed to generate new udev monitoring object for the new client.");
                }
            } else {
                AFB_REQ_ERROR(req,
                "Failed to generate new udev object for the new client.");
            }
        } else {
            AFB_REQ_ERROR(req,"Failed to allocate memory for new client context.");
        }
    }

    return (void*)ctx;
}

static void
pinfo_device_jdev_destructor(struct json_object* jdevice, void* udevice)
{
    UNUSED(jdevice)
    udev_device_unref((struct udev_device*)udevice);
}

static int
pinfo_device_filter_monitoring(pinfo_client_ctx_t* ctx) {
    int res = PINFO_OK;

    if(ctx && ctx->umon_hndl &&
    json_object_is_type(ctx->filter,json_type_object)) {
        json_object* jprop_filter = NULL;
        json_object* jval = NULL;

        if(json_object_object_get_ex(ctx->filter,"properties",&jprop_filter)) {
            const char* subsys_str = NULL;
            const char* devtype_str = NULL;

            if(json_object_object_get_ex(jprop_filter,"SUBSYSTEM",&jval) &&
            json_object_is_type(jval,json_type_string)) {
                subsys_str = json_object_get_string(jval);
            }
            jval = NULL;
            if(json_object_object_get_ex(jprop_filter,"DEVTYPE",&jval) &&
            json_object_is_type(jval,json_type_string)) {
                devtype_str = json_object_get_string(jval);
            }
            if(devtype_str || subsys_str) {
                res |= udev_monitor_filter_add_match_subsystem_devtype(
                        ctx->umon_hndl,subsys_str,devtype_str);
            }
        }

        if(json_object_object_get_ex(ctx->filter,"tags",&jval)) {
            if(json_object_is_type(jval,json_type_array)) {
                const size_t tags_count = json_object_array_length(jval);

                if(tags_count > 0) {
                    int tag_idx = 0;
                    json_object *tag_item = NULL;

                    for(tag_item = json_object_array_get_idx(jval,0);
                        tag_item && tag_idx < tags_count;
                        tag_item = json_object_array_get_idx(jval,++tag_idx)) {

                        if(json_object_is_type(tag_item,json_type_string)) {
                            res |= udev_monitor_filter_add_match_tag(
                                ctx->umon_hndl,
                                json_object_get_string(tag_item)
                            );
                        } else {
                            AFB_WARNING("Client passed invalid tag type,\
                            avoid the tag expression");
                        }
                    }
                } else {
                    //Empty tags array
                }
            } else {
                AFB_WARNING("Client passed invalid value for 'tags' field,\
                the value type should be a json array with json string items,\
                avoid the tags filtering");
            }
        }
    }

    return res;
}


static json_object*
pinfo_device_udevice_to_jlist(  struct udev_device* udevice,
                                struct udev_list_entry*(*udevice_elist)(struct udev_device*),
                                const char*(*elist_val_get)(struct udev_device*,const char*),
                                const unsigned int jflags) {
    json_object* jobject = NULL;

    if(udevice && udevice_elist) {
        struct udev_list_entry* elist = udevice_elist(udevice);
        struct udev_list_entry* elist_head = NULL;

        jobject = json_object_new_object();
        udev_list_entry_foreach(elist_head,elist) {
            const char* skey = udev_list_entry_get_name(elist_head);
            const char* svalue = elist_val_get(udevice,skey);

            if(skey && svalue) {
                json_object_object_add_ex(jobject,skey,json_object_new_string(svalue),jflags);
            }
        }
    }

    return jobject;
}

static json_object*
pinfo_device_udevice_to_jlist_mask( struct udev_device* udevice,
                                    const char*(*udevice_val_getter)(struct udev_device*,const char*),
                                    json_object* jkeys,
                                    unsigned jcpy_flags) {
    json_object* jdev = NULL;

    if(json_object_is_type(jkeys,json_type_array)) {
        const int keys_count = (int) json_object_array_length(jkeys);
        if(keys_count > 0) {
            int key_idx = 0;
            jdev = json_object_new_object();

            for(;key_idx < keys_count; ++key_idx) {
                const char* skey = json_object_get_string(
                        json_object_array_get_idx(jkeys,key_idx));
                const char* svalue = udevice_val_getter(udevice,skey);

                if(svalue) {
                    json_object_object_add_ex(jdev,skey,json_object_new_string(svalue),jcpy_flags);
                }
            }
        }
    }

    return jdev;
}

static json_object*
pinfo_device_udevice_to_jdevice(struct udev_device* udevice, json_object* jmask) {
    json_object *jdev = NULL;

    if(udevice) {
        json_object* jprops = NULL;
        json_object* jattrs = NULL;

        jdev = json_object_new_object();
        if(json_object_is_type(jmask,json_type_object)) {
            json_object* jprops_mask = NULL;
            json_object* jattrs_mask = NULL;

            if(json_object_object_get_ex(jmask,"properties",&jprops_mask)) {
                jprops = pinfo_device_udevice_to_jlist_mask(
                    udevice,
                    udev_device_get_property_value,
                    jprops_mask,
                    JSON_NEW_CONST_KEY
                );
            }
            if(json_object_object_get_ex(jmask,"attributes",&jattrs_mask)) {
                jattrs = pinfo_device_udevice_to_jlist_mask(
                    udevice,
                    udev_device_get_sysattr_value,
                    jattrs_mask,
                    JSON_NEW_CONST_KEY
                );
            }
        }

        if(!jprops) {
            jprops = pinfo_device_udevice_to_jlist(
                udevice,
                udev_device_get_properties_list_entry,
                udev_device_get_property_value,
                JSON_NEW_CONST_KEY
            );
        }

        if(!jattrs) {
            jattrs = pinfo_device_udevice_to_jlist(
                udevice,
                udev_device_get_sysattr_list_entry,
                udev_device_get_sysattr_value,
                JSON_NEW_CONST_KEY
            );
        }

        if(json_object_is_type(jprops,json_type_object) &&
        json_object_object_length(jprops) > 0) {
            json_object_object_add_ex(
                jdev,
                "properties",
                jprops,
                JSON_NEW_CONST_KEY
            );
        }

        if(json_object_is_type(jattrs,json_type_object) &&
        json_object_object_length(jattrs) > 0) {
            json_object_object_add_ex(
                jdev,
                "attributes",
                jattrs,
                JSON_NEW_CONST_KEY
            );
        }

        json_object_set_userdata(jdev,udevice,(json_object_delete_fn*)pinfo_device_jdev_destructor);
    }

    return jdev;
}

static void*
pinfo_device_monitor_loop(pinfo_client_ctx_t* ctx) {
    if(ctx && ctx->umon_hndl) {
        int fd;
        int res = 0;

        res = udev_monitor_enable_receiving(ctx->umon_hndl);

        if(res >= 0) {
            fd = udev_monitor_get_fd(ctx->umon_hndl);
            while (1) {
                int ret;
                fd_set fds;

                FD_ZERO(&fds);
                FD_SET(fd,&fds);

                ret = select(fd+1, &fds, NULL, NULL, NULL);
                if(ret > 0 && FD_ISSET(fd,&fds)) {
                    struct udev_device* detected_dev = NULL;

                    detected_dev = udev_monitor_receive_device(ctx->umon_hndl);
                    if(detected_dev) {
                        json_object* jdetected_dev = NULL;
                        jdetected_dev = pinfo_device_udevice_to_jdevice(detected_dev,ctx->mask);
                        if(jdetected_dev) {
                            ctx->umon_cb(ctx,jdetected_dev);
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

static void
pinfo_device_filter_scan(struct udev_enumerate* udev_enum, json_object* jfilter) {
    if(udev_enum && jfilter) {
        if(json_object_is_type(jfilter,json_type_object)) {
            json_object* jval = NULL;

			if(json_object_object_get_ex(jfilter,"tags",&jval)) {
				if(json_object_is_type(jval,json_type_array)) {
					int tag_idx = 0;
					const int tags_count = (int) json_object_array_length(jval);

					if(tags_count > 0) {
                        json_object* jtag = NULL;

                        for(jtag = json_object_array_get_idx(jval,0);
                            jtag && tag_idx < tags_count;
                            jtag = json_object_array_get_idx(jtag,++tag_idx)) {
                            udev_enumerate_add_match_tag(udev_enum,json_object_get_string(jtag));
                        }
					} else {
						//Empty json array for tags array
					}
				} else if(json_object_is_type(jval,json_type_string)) {
					udev_enumerate_add_match_tag(udev_enum,json_object_get_string(jval));
				} else {
                    AFB_WARNING("Client passed invalid value for 'tags' field,\
                    the value type should be a json array with json string items\
                    or a json string, avoid the tags filtering");
				}
			}

			if(json_object_object_get_ex(jfilter,"properties",&jval) &&
            json_object_is_type(jval,json_type_object)) {
                if(json_object_object_length(jval) > 0) {
                    json_object_object_foreach(jval,key,value) {
                        udev_enumerate_add_match_property(udev_enum,key,json_object_get_string(value));
                    }
                }
			}

			if(json_object_object_get_ex(jfilter,"attributes",&jval) &&
            json_object_is_type(jval,json_type_object)) {
                if(json_object_object_length(jval) > 0) {
                    json_object_object_foreach(jval,key,value) {
                        udev_enumerate_add_match_sysattr(udev_enum,key,json_object_get_string(value));
                    }
                }
			}
        }
    }
}

json_object*
pinfo_device_scan(json_object *jfilter, json_object* jmask) {
    json_object* jdevs_arr = NULL;
    struct udev* udev_ctx = NULL;

    udev_ctx = udev_new();
    jdevs_arr = json_object_new_array();

    if(udev_ctx) {
        struct udev_enumerate *dev_enum = NULL;

        dev_enum = udev_enumerate_new(udev_ctx);
        if(dev_enum) {
            struct udev_list_entry *dev_elist = NULL;
            struct udev_list_entry *dev_elist_head = NULL;
            pinfo_device_filter_scan(dev_enum,jfilter);

            udev_enumerate_scan_devices(dev_enum);
            dev_elist = udev_enumerate_get_list_entry(dev_enum);
            if(dev_elist) {
                struct udev_device *udevice = NULL;
                struct json_object *jdevice = NULL;
                int udev_num = 0;
                int jdev_num = 0;

                udev_list_entry_foreach(dev_elist_head,dev_elist) {
                    const char* path = udev_list_entry_get_name(dev_elist_head);
                    udev_num++;
                    udevice = udev_device_new_from_syspath((struct udev*)udev_ctx,path);
                    jdevice = pinfo_device_udevice_to_jdevice(udevice,jmask);
                    if(jdevice) {
                        json_object_array_add(jdevs_arr,jdevice);
                        jdev_num++;
                    } else {
                        udev_device_unref(udevice);
                    }
                }
                AFB_INFO("[SCAN]: %d device detected, %d device info reported back.",udev_num,jdev_num);
            } else {
                AFB_WARNING("No device found or enumeration failed");
            }
        } else {
            AFB_DEBUG("Unable to allocate enumeration object.");
        }
        udev_enumerate_unref(dev_enum);
    }

    return jdevs_arr;
}
