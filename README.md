# pico-w-connection-manager

A Pico W C++ class for access point scan, connect, disconnect, & RSSI with
flash-stored lists of known SSIDs and passwords 

This class is designed to be included as a git submodule within a larger project.

For an example of how to use it, and where to install the source code in your
project directory structure, see the [demo program](https://github.com/rppicomidi/pico-w-connection-manager-demo).

# Dependencies
Aside from the dependencies on the Pico C/C++ SDK and the LwIP
library the SDK includes, this class uses the following external code:

- the `parson` JSON library to serialize and deserialize settings to JSON format
- the `littlefs-lib` file system to store Wi-Fi settings in JSON format to
a small reserved amount of Pico board program flash.
- the `main_lwipopts.h` include file for the main project. This file is the
`lwipopts.h` file used by the application that is using the `Pico-w-connection-manager`
class. It must be installed 2 directory levels up from the `pico-w-connection-manager`
directory, and it must be called `main_lwipopts.h`. See `lwipopts.h` in this project
for more details.

# Installation
Get this source code as a submodule within the same subdirectory as the `parson` library
and the `littlefs-lib` library. For example, if your project source code is in `${proj_dir}`
and if `littlefs-lib` and `parson` are installed `${proj_dir}/lib`

```
cd ${proj_dir}/lib
git submodule add https://github.com/rppicomidi/pico-w-connection-manager.git
```


# Known Issues
For all known issues, check the date. By the time you build this, they
may be fixed.
## On 23-jan-2023
The Pico-W always fails to connect to an open (no password) access point. This was
fixed in the develop branch of the pico-sdk 3 days ago. See below how to update the SDK.
(see [this pull requst](https://github.com/raspberrypi/pico-sdk/pull/1181)) if you are
interested)
## On 6-dec-2022
If you call `initialize()` after you call `deinitialize()` then the software will hang up.
This is an issue the `pico-sdk`. To work around this issue, use the
`pico-sdk` `development` branch and patch per the discussion in [sdk issue #980](https://github.com/raspberrypi/pico-sdk/issues/980):

```
cd ${PICO_SDK_PATH}
git fetch origin
git checkout -b develop origin/develop
git submodule update lib/cyw43-driver/
```

Edit the file `pico-sdk/src/rp2_common/pico_cyw43_archcyw43_arch_threadsafe_background.c`. 
Replace code near line 194

```
#if CYW43_LWIP
    lwip_init();
#endif
```

with

```
#if CYW43_LWIP
    static bool done_lwip_init;
    if (!done_lwip_init) {
        lwip_init();
        done_lwip_init = true;
    }
#endif
```

