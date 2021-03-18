###########################################################################
# Copyright 2015, 2016, 2018 IoT.bzh
#
# author: Fulup Ar Foll <fulup@iot.bzh>
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
set(PROJECT_AUTHOR "LEFEBVRE Valentin")
set(PROJECT_AUTHOR_MAIL "valentin.lefebvre@iot.bzh")
set(PROJECT_LICENSE "APL2.0")
set(PROJECT_LANGUAGES "C")

# Which directories inspect to find CMakeLists.txt target files
# set(PROJECT_SRC_DIR_PATTERN "*")

# Where are stored the project configuration files
# relative to the root project directory
set(PROJECT_CMAKE_CONF_DIR "conf.d")

# Compilation Mode (DEBUG, RELEASE)
# ----------------------------------
set(BUILD_TYPE "DEBUG")
set(CMAKE_BUILD_TYPE "DEBUG")

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
#set (kernel_mandatory_version 4.8)
set (kernel_minimal_version 4.8)

# Compiler selection if needed. Impose a minimal version.
# -----------------------------------------------
set (gcc_minimal_version 4.9)

# PKG_CONFIG required packages
# -----------------------------
set (PKG_REQUIRED_LIST
	json-c>=0.13
	afb-binding
	afb-helpers
)

# You can also consider to include libsystemd
# -----------------------------------
#list (APPEND PKG_REQUIRED_LIST libsystemd>=222)

# Prefix path where will be installed the files
# Default: /usr/local (need root permission to write in)
# ------------------------------------------------------
set(INSTALL_PREFIX  $ENV{HOME}/opt)
set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX}/lib64/pkgconfig ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
set(LD_LIBRARY_PATH ${CMAKE_INSTALL_PREFIX}/lib64 ${CMAKE_INSTALL_PREFIX}/lib)


# Customize link option
# -----------------------------
#list(APPEND link_libraries -an-option)

# Compilation options definition
# Use CMake generator expressions to specify only for a specific language
# Values are prefilled with default options that is currently used.
# Either separate options with ";", or each options must be quoted separately
# DO NOT PUT ALL OPTION QUOTED AT ONCE , COMPILATION COULD FAILED !
# ----------------------------------------------------------------------------
#set(COMPILE_OPTIONS
# -Wall
# -Wextra
# -Wconversion
# -Wno-unused-parameter
# -Wno-sign-compare
# -Wno-sign-conversion
# -Werror=maybe-uninitialized
# -Werror=implicit-function-declaration
# -ffunction-sections
# -fdata-sections
# -fPIC
# CACHE STRING "Compilation flags")
#set(C_COMPILE_OPTIONS "" CACHE STRING "Compilation flags for C language.")
#set(CXX_COMPILE_OPTIONS "" CACHE STRING "Compilation flags for C++ language.")
#set(PROFILING_COMPILE_OPTIONS
# -g
# -O0
# -pg
# -Wp,-U_FORTIFY_SOURCE
# CACHE STRING "Compilation flags for PROFILING build type.")
#set(DEBUG_COMPILE_OPTIONS
# -g
# -ggdb
# CACHE STRING "Compilation flags for DEBUG build type.")
#set(COVERAGE_COMPILE_OPTIONS
# -g
# -O0
# --coverage
# CACHE STRING "Compilation flags for COVERAGE build type.")
#set(RELEASE_COMPILE_OPTIONS
# -O2
# -pipe
# -D_FORTIFY_SOURCE=2
# -fstack-protector-strong
# -Wformat -Wformat-security
# -Werror=format-security
# -feliminate-unused-debug-types
# -Wl,-O1
# -Wl,--hash-style=gnu
# -Wl,--as-needed
# -fstack-protector-strong
# -Wl,-z,relro,-z,now
# CACHE STRING "Compilation flags for RELEASE build type.")

add_definitions(-DAFB_BINDING_VERSION=3)

# Compilation options definition
set(SCRIPTS_PATH "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/var/" CACHE STRING "SCRIPTS_PATH")
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
