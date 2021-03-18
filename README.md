# platform-info-binding


Previously used for AGL project, this binding reads system and platform information and expose them through
its API. The binding reads all files within /etc/platform-info directory and try
to serialize them as JSON. Then you could access to a JSON object using a
doted path.

i.e: **build.machine.model** or **build.id**

Moreover, it is possible to get more specifics informations in adding scripts which return informations to put on JSON. Scripts have to return a correct json object.

## Build for 'native' Linux distros (Fedora, openSUSE, Debian, Ubuntu, ...)

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Get verb

```bash
afb-client ws://localhost:1234/api 
platform-info get "soc_vendor"
ON-REPLY 6:platform-info/get: OK
{
  "jtype":"afb-reply",
  "request":{
    "status":"success"
  },
  "response":"Nitrogen"
}
```

```bash
afb-client ws://localhost:1234/api 
platform-info get "soc_vendor"
ON-REPLY 6:platform-info/get: OK
{
  "jtype":"afb-reply",
  "request":{
    "status":"success"
  },
  "response":"Nitrogen"
}
```

```bash
platform-info get "packages_installed"
ON-REPLY 3:platform-info/get: OK
{
  "jtype":"afb-reply",
  "request":{
    "status":"success"
  },
  "response":[
    {
      "acl.x86_64":"2.2.53-9.fc33"
    },
    {
      "afb-binder.x86_64":"4.0.0beta2-26.4"
    },
    {
      "afb-binding-devel.x86_64":"4.0.0beta2-23.4"
    },
    {
      "afb-client.x86_64":"4.0.0beta3-23.4"
    },
    {
      "afb-cmake-modules.noarch":"10.0.0-75.4"
    },
    {
      "afb-idl.x86_64":"0.1-6.4"
    },
    [...]
}
```