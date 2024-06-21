/*
 * Copyright (C) 2016, 2018 "IoT.bzh"
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

#define AFB_BINDING_VERSION 4
#include <afb-helpers4/afb-req-utils.h>
#include <afb/afb-binding.h>
#include <rp-utils/rp-jsonc.h>

// clang-format off
const char * all_info[] =  {
	"gpu_name",
  	"soc_family",
	"cpu_arch",
  	"soc_name",
  	"cpu_cache_kb",
  	"board_model",
  	"cpu_count",
  	"cpu_compatibility",
  	"memory_total_mb",
  	"cpu_freq_mhz",
  	"soc_id",
  	"soc_revision",
  	"soc_vendor",
  	"os_version",
  	"os_name",
  	"ethernet_devices",
	"bluetooth_devices",
  	"wifi_devices",
  	"can_devices",
	NULL
};
// clang-format on

struct key_search_t
{
    afb_api_t api;
    json_object **result;
};

json_object *platform_info;
