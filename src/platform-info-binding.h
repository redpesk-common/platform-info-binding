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

#include <wrap-json.h>
#include "platform-info-apidef.h"

const char * all_info[11] =  {
	"os_name",
	"os_version",
	"soc_vendor",
	"soc_revision",
	"soc_family",
	"soc_id",
	"cpu_count",
	"board_model",
	"afm_packages_installed",
	"packages_number",
	"packages_installed"
};

struct key_search_t {
	afb_api_t api;
	json_object **result;
};

json_object *platform_info;
