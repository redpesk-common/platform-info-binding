#!/bin/bash

set -e

###########################################################################
# Copyright 2019 IoT.bzh
#
# author:Frédéric Marec <frederic.marec@iot.bzh>
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

if [ ! -d "/etc/platform-info" ]
then
    echo "missing directory /etc/platform-info"
    exit 0
else
    if [ ! -f "/etc/platform-info/build.json" ] && [ ! -f "/etc/platform-info/hardware.json" ]
    then
        echo "missing tests files for testing"
        exit 0
    else
        exit 1
    fi
fi