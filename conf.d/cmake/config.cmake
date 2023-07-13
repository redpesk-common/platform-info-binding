###########################################################################
# Copyright 2015, 2016, 2018 IoT.bzh
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

# Project Info
# ------------------
set(PROJECT_NAME "platform-info-binding")
set(PROJECT_VERSION 1.0)
set(API_NAME platform-info)
set(PROJECT_PRETTY_NAME "Platform Information provider binding")
set(PROJECT_DESCRIPTION "A binding meant to provide system, platform, build information for others bindings and apps")
set(PROJECT_URL "http://git.ovh.iot/redpesk/redpesk-common/platform-info-binding/")
set(PROJECT_ICON "icon.png")
set(PROJECT_AUTHOR "IoT.bzh Team")
set(PROJECT_AUTHOR_MAIL "team@iot.bzh")
set(PROJECT_LICENSE "APL2.0")
set(PROJECT_LANGUAGES "C")

# Which directories inspect to find CMakeLists.txt target files
# set(PROJECT_SRC_DIR_PATTERN "*")

# Where are stored the project configuration files
# relative to the root project directory
set(PROJECT_CMAKE_CONF_DIR "conf.d")

# Kernel selection if needed. You can choose between a
# mandatory version to impose a minimal version.
# Or check Kernel minimal version and just print a Warning
# about missing features and define a preprocessor variable
# to be used as preprocessor condition in code to disable
# incompatibles features. Preprocessor define is named
# KERNEL_MINIMAL_VERSION_OK.
#
# NOTE*** FOR NOW IT CHECKS KERNEL Yocto environment and
# Yocto SDK Kernel version.
# -----------------------------------------------
# PKG_CONFIG required packages
# -----------------------------
set (PKG_REQUIRED_LIST
	json-c
	afb-binding
	afb-helpers
)

# You can also consider to include libsystemd
# -----------------------------------
#list (APPEND PKG_REQUIRED_LIST libsystemd>=222)

# Customize link option
# -----------------------------
#list(APPEND link_libraries -an-option)

add_definitions(-DAFB_BINDING_VERSION=3)

# Compilation options definition
set(SCRIPTS_PATH "${AFM_APP_DIR}/${PROJECT_NAME}/var" CACHE STRING "SCRIPTS_PATH")
add_definitions(-DSCRIPTS_PATH="${SCRIPTS_PATH}")

# Location for config.xml.in template file.
#
# If you keep them commented then it will build with a default minimal widget
# template which is very simple and it is highly probable that it will not suit
# to your app.
# -----------------------------------------
#set(WIDGET_ICON "conf.d/wgt/${PROJECT_ICON}" CACHE PATH "Path to the widget icon")
set(WIDGET_CONFIG_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/conf.d/wgt/config.xml.in" CACHE PATH "Path to widget config file template (config.xml.in)")
#set(TEST_WIDGET_CONFIG_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/conf.d/wgt/test-config.xml.in" CACHE PATH "Path to the test widget config file template (test-config.xml.in)")

# Mandatory widget Mimetype specification of the main unit
# --------------------------------------------------------------------------
# Choose between :
#- text/html : HTML application,
#	content.src designates the home page of the application
#
#- application/vnd.agl.native : AGL compatible native,
#	content.src designates the relative path of the binary.
#
# - application/vnd.agl.service: AGL service, content.src is not used.
#
#- ***application/x-executable***: Native application,
#	content.src designates the relative path of the binary.
#	For such application, only security setup is made.
#
set(WIDGET_TYPE application/vnd.agl.service)

# Mandatory Widget entry point file of the main unit
# --------------------------------------------------------------
# This is the file that will be executed, loaded,
# at launch time by the application framework.
#
set(WIDGET_ENTRY_POINT "lib/afb-platform-info.so")

# Optional dependencies order
# ---------------------------
#set(EXTRA_DEPENDENCIES_ORDER)

# Optional Extra global include path
# -----------------------------------
#set(EXTRA_INCLUDE_DIRS)

# Optional extra libraries
# -------------------------
#set(EXTRA_LINK_LIBRARIES)

# Optional force binding Linking flag
# ------------------------------------
# set(BINDINGS_LINK_FLAG LinkOptions )

# Optional force package prefix generation, like widget
# -----------------------------------------------------
# set(PKG_PREFIX DestinationPath)

# Optional Application Framework security token
# and port use for remote debugging.
#------------------------------------------------------------
set(AFB_TOKEN   ""     CACHE PATH "Default binder security token")
set(AFB_REMPORT "1234" CACHE PATH "Default binder listening port")

# Print a helper message when every thing is finished
# ----------------------------------------------------
set(CLOSING_MESSAGE "platform-info-binding launch: SCRIPTS_PATH=../scripts/scripts afb-binder --port=1234 --name=afb-platform-info --workdir=${CMAKE_BINARY_DIR}/package --ldpaths=${CMAKE_BINARY_DIR} --roothttp=htdocs  -vvv")
set(PACKAGE_MESSAGE "Install widget file using in the target : afm-util install ${PROJECT_NAME}.wgt")

# Optional schema validator about now only XML, LUA and JSON
# are supported
#------------------------------------------------------------
#set(LUA_CHECKER "luac" "-p" CACHE STRING "LUA compiler")
#set(XML_CHECKER "xmllint" CACHE STRING "XML linter")
#set(JSON_CHECKER "json_verify" CACHE STRING "JSON linter")

include(CMakeAfbTemplates)
