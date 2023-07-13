###########################################################################
# Copyright 2021 IoT.bzh
#
# author: Valentin Lefebvre <valentin.lefebvre@iot.bzh>
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

PROJECT_TARGET_ADD(scripts)

configure_file(afm_packages_installed.sh.in afm_packages_installed.sh @ONLY)

set(SCRIPT_FILES
	${CMAKE_CURRENT_BINARY_DIR}/afm_packages_installed.sh
	${CMAKE_CURRENT_SOURCE_DIR}/packages_installed.sh
	${CMAKE_CURRENT_SOURCE_DIR}/packages_number.sh
)

add_input_files("${SCRIPT_FILES}")

SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES 
    LABELS "DATA"
    OUTPUT_NAME ${TARGET_NAME}
)