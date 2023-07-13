###########################################################################
# Copyright 2015, 2016, 2017 IoT.bzh
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###########################################################################

# Add target to project dependency list
PROJECT_TARGET_ADD(platform-info)

    # Add generated file
    SET(JSON_INFO_C ${CMAKE_CURRENT_BINARY_DIR}/json_info.c)

    # Define project Targets
    ADD_LIBRARY(${TARGET_NAME} SHARED platform-info-binding.c ${JSON_INFO_C})

    # Binder exposes a unique public entry point
    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
        PREFIX "afb-"
        LABELS "BINDING"
        OUTPUT_NAME ${TARGET_NAME}
    )

    # Generate c file for the info verb
    ADD_CUSTOM_COMMAND(
        OUTPUT json_info.c
        COMMAND echo 'const char * info_verbS=\"\\' > ${JSON_INFO_C}
        COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/info_verb.json | sed -e 's/$$/\\\\/' -e 's/\"/\\\\\"/g' >> ${JSON_INFO_C}
        COMMAND echo '\"\;' >> ${JSON_INFO_C}
        DEPENDS info_verb.json
    )
