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

cmake_minimum_required(VERSION 3.12)
project(platform-info-binding VERSION 9.0.0 LANGUAGES C)

# Add target to project dependency list
set(TARGET_NAME platform-info-binding)

# Add target install directory 
set(AFM_APP_DIR ${CMAKE_INSTALL_PREFIX}/redpesk CACHE PATH "Applications directory")
set(APP_DIR ${AFM_APP_DIR}/${PROJECT_NAME})

# getting scripts
set(SCRIPTS_PATH "${APP_DIR}/var" CACHE STRING "SCRIPTS_PATH")

configure_file(scripts/afm_packages_installed.sh.in ${CMAKE_CURRENT_BINARY_DIR}/scripts/afm_packages_installed.sh @ONLY)

# Add generated file
set(JSON_INFO_C ${CMAKE_CURRENT_BINARY_DIR}/json_info.c)

# Define project Targets
add_library(${TARGET_NAME} SHARED src/platform-info-binding.c src/platform-info-devices.c ${JSON_INFO_C})

# Check dependencies
include(FindPkgConfig)
pkg_check_modules(deps REQUIRED
    afb-binding
    afb-helpers4
    libudev
)

# Binder exposes a unique public entry point
set_target_properties(${TARGET_NAME} PROPERTIES
    PREFIX ""
    LABELS "BINDING"
    OUTPUT_NAME ${TARGET_NAME}
)
target_link_libraries(${TARGET_NAME} PRIVATE ${deps_LINK_LIBRARIES})

# Generate c file for the info verb
add_custom_command(
    OUTPUT json_info.c
    COMMAND echo 'const char * info_verbS=\"\\' > ${JSON_INFO_C}
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/src/info_verb.json | sed -e 's/$$/\\\\/' -e 's/\"/\\\\\"/g' >> ${JSON_INFO_C}
    COMMAND echo '\"\;' >> ${JSON_INFO_C}
    DEPENDS src/info_verb.json
)

# Install
install(TARGETS ${TARGET_NAME} DESTINATION ${APP_DIR}/lib)
install(FILES manifest.yml DESTINATION ${APP_DIR}/.rpconfig)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/afm_packages_installed.sh 
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/packages_installed.sh 
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/packages_number.sh
    DESTINATION ${APP_DIR}/var
)
