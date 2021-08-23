# Usage Guide

## I - Creation JSON files

**platform-runtime-tools** will automatically generate all files into the directory **/etc/platform-info/** when it will be installed. However, in case of you want to refresh files or something wrong happens with the file generations. You can manually call services to get appropriate information files. You will have two information parts:

### a. Core Information

```bash
sudo systemctl start platform-core-detection
```

Result:

```bash
$ cat /etc/platform-info/core.json 
{"soc_id":"i7-8550U","gpu_name":"","memory_total_mb":970,"cpu_freq_mhz":"","soc_family":"","soc_revision":10,"cpu_count":1,"cpu_cache_kb":8480,"cpu_arch":"x86_64","board_model":"unknown","cpu_compatibility":"unknown","soc_name":"Intel i7-8550U","soc_vendor":"intel"}
```

### b. Os information

```bash
sudo systemctl start platform-os-detection
```

Result:

```bash
$ cat /etc/platform-info/os.json 
{"os_name":"Fedora","os_version":33}
```

## II - Usage of the API verbs

### a. Run the binding

To run this binding:

```bash
afb-binder --port=8001 --name=afb-platform-info --workdir=/var/local/lib/afm/applications/platform-info-binding --binding=lib/afb-platform-info.so -vvv
```

### b. API verbs

#### get

This API has a verb called *'get'*. At this verbs you will want to add your specific information requests. For example if you need to know the os version:

```bash
afb-client ws://localhost:8001/api 
platform-info get "os_version"
ON-REPLY 0:platform-info/get: OK
{
  "jtype":"afb-reply",
  "request":{
    "status":"success"
  },
  "response":33
}
```

Here is a list of what you can ask:

* "os_name"
* "os_version"
* "soc_vendor"
* "soc_family"
* "soc_id"
* "cpu_count"
* "board_model"
* "packages_afm_installed"
* "packages_number"
* "packages_installed"

#### get_all_info

And you have a verb called *'get_all_info'* that return a set of useful system information:

```bash
afb-client ws://localhost:8001/api
platform-info get_all_info 
ON-REPLY 0:platform-info/get_all_info: OK 
{
  "jtype": "afb-reply",
  "request": {
    "status": "success"
  },
  "response": {
    "os_name": "Fedora",
    "os_version": 33,
    "soc_vendor": "intel",
    "soc_revision": 10,
    "soc_family": "",
    "soc_id": "i7-8550U",
    "cpu_count": 1,
    "board_model": "unknown",
    "afm_packages_installed": [
      {
        "canbus-binding": "9.99.1+20201222+6+g2a919fc-29.23"
      },
      {
        "canopen-binding": "1.1.0+20201215+1+gab54d92-12.37"
      },
      {
        "platform-info-binding": "1.1.0-10.1"
      },
      {
        "redis-tsdb-binding": "1.0.1-21.8"
      },
      {
        "signal-composer-binding": "10.0.4-37.30"
      },
      {
        "spawn-binding": "0.0.0+20210126+164049+0+g4abfbbbe-16.23"
      }
    ],
    "packages_number": [
      {
        "numbers": 672
      }
    ],
    "packages_installed": [
      {
        "NetworkManager.x86_64": "1:1.26.6-1.fc33"
      },
      {
        "NetworkManager-libnm.x86_64": "1:1.26.6-1.fc33"
      },
      {
        "acl.x86_64": "2.2.53-9.fc33"
      },
      {
        "afb-binder.x86_64": "4.0.0beta5-29.2"
      },
      {
        "afb-binding-devel.x86_64": "4.0.0beta5-33.7"
      },
[...]
  }
}
```
