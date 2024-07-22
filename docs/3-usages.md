# Usage Guide

## I - Creation JSON files

**platform-runtime-tools** will automatically generate all files into the directory **/etc/platform-info/** when it is installed. However, in case you want to refresh files or something wrong happens with the file generations. You can manually call services to get appropriate information files. You will have two information parts:

### a. Core Information

```bash
sudo systemctl start platform-core-detection
```

```bash
$ cat /etc/platform-info/core.json 
{"soc_id":"i7-8550U","gpu_name":"","memory_total_mb":970,"cpu_freq_mhz":"","soc_family":"","soc_revision":10,"cpu_count":1,"cpu_cache_kb":8480,"cpu_arch":"x86_64","board_model":"unknown","cpu_compatibility":"unknown","soc_name":"Intel i7-8550U","soc_vendor":"intel"}
```

### b. Os information

```bash
sudo systemctl start platform-os-detection
```

```bash
$ cat /etc/platform-info/os.json 
{"os_name":"Fedora","os_version":40}
```

## II - Usage of the API verbs

### a. Run the binding

To run this binding from build directory:

```bash
SCRIPTS_PATH=../scripts/ afb-binder --binding=./platform-info-binding.so -vvv
```

#### Start the web client

After starting the binder start the web client in a new tab with:

```bash
afb-client ws://localhost:8001/api
```

### b. API verbs

#### get

This API has a verb called *'get'*. At this verb you will want to add your specific information requests. For example if you need to know the os version:

```bash
platform-info get "os_version"
```

Respond:

```json
{"jtype":"afb-reply","request":{"status":"success","code":0},"response":"40 (Workstation Edition)"}
```

Here is a list of what you can ask:

* "gpu_name",
* "soc_family",
* "cpu_arch",
* "soc_name",
* "cpu_cache_kb",
* "board_model",
* "cpu_count",
* "cpu_compatibility",
* "memory_total_mb",
* "cpu_freq_mhz",
* "soc_id",
* "soc_revision",
* "soc_vendor",
* "os_version",
* "os_name",
* "ethernet_devices",
* "bluetooth_devices",
* "wifi_devices",
* "can_devices",

#### get_all_info

You have a verb called *'get_all_info'* that returns a set of useful system information:

```bash
platform-info get_all_info
```

Respond:

```json
platform-info get_all_info 
{"jtype":"afb-reply",
"request":{"status":"success","code":0},
  "response":{"gpu_name":"Lenovo Device 225d",
              "soc_family":"Core i7",
              "cpu_arch":"unknown",
              "soc_name":"Intel Core i7 i7-8550U",
              "cpu_cache_kb":10496,
              "board_model":"unknown",
              "cpu_count":8,
              "cpu_compatibility":"unknown",
              "memory_total_mb":15748,
              "cpu_freq_mhz":2000,
              "soc_id":"i7-8550U",
              "soc_revision":10,
              "soc_vendor":"intel",
              "os_version":"40 (Workstation Edition)",
              "os_name":"Fedora Linux",
              "ethernet_devices":"enp0s20f0u2u4u4 enp0s31f6",
              "bluetooth_devices":"",
              "wifi_devices":"wlp3s0",
              "can_devices":""
            }
}
```

#### subscribe

The `subscribe` verb will allow you to detect and get information when an `udev` event is emitted (e.g. when a new USB device is plugged in).
You can add a filter on events you want to register to, by specifying which `udev` SUBSYSTEM, DEVTYPE or TAGS (TAGS are added by udev rules created by the user so if you didn't made any there will only be a few defaults tags).

```bash
platform-info subscribe {content of request listed below}
```

```json
platform-info subscribe {"event":"monitor-devices"}
                        {"event":"monitor-devices","filter": {"properties": {"SUBSYSTEM": "input"}}}
                        {"event":"monitor-devices","filter": {"properties": {"SUBSYSTEM": "disk", "DEVTYPE": "disk"}}}
                        {"event":"monitor-devices","filter":"tags"}
                        {"event":"monitor-devices","mask":{"properties":["properties1", "properties2", "properties3"], "attributes":["attributes1", "attributes2"]}}
```

#### unsubscribe

Allows you to `unsubscribe` from an event

```bash
platform-info unsubscribe {same request as the one you subscribed to}
```

```json
platform-info unsubscribe {"event":"monitor-devices"}
                          {"event":"monitor-devices","filter": {"properties": {"SUBSYSTEM": "input"}}}
                          {"event":"monitor-devices","filter": {"properties": {"DEVTYPE": "mydevtype"}}}
                          {"event":"monitor-devices","filter":"tags"}
                          {"event":"monitor-devices","mask":{"properties":["properties1", "properties2", "properties3"], "attributes":["attributes1", "attributes2"]}}
```
