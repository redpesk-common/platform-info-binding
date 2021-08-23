# Installation guide

## I - prerequisite

To be able to use correctly this binding you need to install first the tool which will create all important files at the path **/etc/platform-info**: **platform-runtime-tools**

* Ubuntu / Debian

    ```bash
    sudo apt install platform-runtime-tools
    ```

* Fedora / OpenSuse

    ```bash
    sudo dnf install platform-runtime-tools
    ```

## II - Using distribution packager manager

Add the Redpesk repository into your packager manager following this link: <download.redpesk.bzh>

Install the package **platform-info-binding**

## III - Building from source

### a. dependencies

Building tools:

* gcc
* g++
* make
* cmake
* afb-cmake-modukes

Then the followings dependencies:

* json-c
* afb-binding
* libmicrohttpd
* afb-libhelpers

#### Fedora / OpenSuse

```bash
sudo dnf install gcc-c++ make cmake afb-cmake-modules json-c-devel afb-binding-devel libmicrohttpd-devel afb-libhelpers-devel
```

#### Ubuntu / debian

```bash
sudo apt install gcc g++ make cmake afb-cmake-modules-bin libjson-c-dev afb-binding-dev libmicrohttpd-dev afb-libhelpers-dev
```

### b. build

```bash
git clone http://git.ovh.iot/redpesk/redpesk-common/platform-info-binding.git
cd platform-info-binding
mkdir build
cd build
cmake ..
make
```
