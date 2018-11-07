# agl-service-platform-info

AF binding that reads system and platform information and expose them through
its API. The binding reads all files within /etc/platform-info directory and try
to serialize them as JSON. Then you could access to a JSON object using a
doted path.

i.e: **build.machine.model** or **build.id**

## Setup

```bash
git clone --recursive https://github.com/iotbzh/agl-service-platform-info
cd agl-service-platform-info
#setup your build environement
. /xdt/sdk/environment-setup-aarch64-agl-linux
#build your application
./conf.d/autobuild/agl/autobuild package
```

## Build for 'native' Linux distros (Fedora, openSUSE, Debian, Ubuntu, ...)

```bash
./conf.d/autobuild/linux/autobuild package
```

## Deploy

### AGL

```bash
export YOUR_BOARD_IP=192.168.1.X
export APP_NAME=agl-service-platform-info
scp build/${APP_NAME}.wgt root@${YOUR_BOARD_IP}:/tmp
ssh root@${YOUR_BOARD_IP} afm-util install /tmp/${APP_NAME}.wgt
APP_VERSION=$(ssh root@${YOUR_BOARD_IP} afm-util list | grep ${APP_NAME}@ | cut -d"\"" -f4| cut -d"@" -f2)
ssh root@${YOUR_BOARD_IP} afm-util start ${APP_NAME}@${APP_VERSION}
```

## Usage

### Get verb

```bash
afb-client-demo ws://localhost:1234/api?token= platform-info get ".build.layers.agl-manifest.revision"
{"response":"7383840-dirty","jtype":"afb-reply","request":{"status":"success","uuid":"3ab64378-44ac-45d1-ad1a-9db7c6c33545"}}
```
