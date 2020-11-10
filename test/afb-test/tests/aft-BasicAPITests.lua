--[[
   Copyright (C) 2019 "IoT.bzh"
   Author Frédéric Marec <frederic.marec@iot.bzh>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


   NOTE: strict mode: every global variables should be prefixed by '_'
--]]

local testPrefix ="agl-info_BasicAPITest_"

_AFT.setBeforeAll(function()

  if not os.execute("/bin/bash ".._AFT.bindingRootDir.."/var/testPresence.sh") then

    print("Platform-info environment already set up")
    return 0

  else
    if not os.execute("/bin/bash ".._AFT.bindingRootDir.."/var/createPlatformInfoDirectory.sh") then

      print("Fail to set up Platform-info test environment")
      return -1

    else
      print("Platform-info environment set up")
      _AFT.setAfterAll(function()
        if not os.execute("/bin/bash ".._AFT.bindingRootDir.."/var/removePlatforInfoDirectory.sh") then

          print("Fail to set down Platform-info test environment")
          return -1

        else
          print("Platform-info environment set down")
          return 0

        end
      end)
      return 0
    end
  end
end)

local ev_mon_arg = "monitor-devices"

-- This tests the 'get' verb of the platform-info API
_AFT.testVerbStatusSuccess(testPrefix.."get", "platform-info", "get", ".layers.agl-manifest")

-- This tests the 'get' verb of the platform-info API
_AFT.testVerbStatusSuccess(testPrefix.."get", "platform-info", "get", {})

-- This tests the 'set' verb of the platform-info API
_AFT.testVerbStatusSuccess(testPrefix.."set", "platform-info", "set", {arg=".build.layers.agl-manifest.revision", value="test"})

-- This tests the 'subscribe' verb of the platform-info API for monitoring devices
_AFT.testVerbStatusSuccess(testPrefix.."subscribe", "platform-info", "subscribe", {event=ev_mon_arg})

-- This tests the 'unsubscribe' verb of the platform-info API for monitoring devices
_AFT.testVerbStatusSuccess(testPrefix.."unsubscribe", "platform-info", "unsubscribe", {event=ev_mon_arg})

-- This tests the 'scan' verb of the platform-info API
_AFT.testVerbStatusSuccess(testPrefix.."scan", "platform-info", "scan", {})