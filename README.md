ESP-IDF component for [eshetcpp](https://github.com/tomjnixon/eshetcpp).

Have a look at [the example](example#readme) to see how this normally all fits
together.

## Using it in an existing ESP-IDF project

Add this repository as a submodule in `components`:

    git submodule add git@github.com:tomjnixon/eshet-esp-idf.git components/eshet

Add `eshet` to your `main` component's `PRIV_REQUIRES`; in `main/CMakeLists.txt` this looks like:

    idf_component_register(SRCS "main.cpp" PRIV_REQUIRES eshet)

Enable C++ exceptions by adding this to `sdkconfig.defaults`:

    CONFIG_COMPILER_CXX_EXCEPTIONS=y

(is there a way to require that this is set from a component?)

## Configuration

If using `get_default_hostport()` to get the ESHET host and port to connect to,
configure the host and port in sdkconfig.defaults:

    CONFIG_ESHET_HOST="server.lan"
    CONFIG_ESHET_PORT=11236

## OTA Updates

This project includes support for sending OTA updates over ESHET -- completely
insecure by default (configuring [signed app
updates](https://docs.espressif.com/projects/esp-idf/en/v5.1.2/esp32/security/secure-boot-v1.html#signed-app-verification-without-hardware-secure-boot)
is left as an exercise for the reader), but convenient fast, and fun.

To enable it, instantiate an `eshet::OTAHandler`:

```cpp
// add this include
#include "eshet_ota.hpp"

// in app_main, after creating the client
OTAHandler ota(client, ESHET_OTA_PATH);
```

This will register the required actions under `ESHET_OTA_PATH`. This is
configured in your project root `CMakeLists.txt` with:

```cmake
setup_ota(/my/device/ota)
```

You will also need to configure partitioning and the bootloader; add these to
`sdkconfig.defaults`:

    CONFIG_PARTITION_TABLE_TWO_OTA=y
    CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y
    CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y

(i'm not sure about the last one, but it's what I use)

Finally, you will need to have built and installed the `eshet_ota` tool from
this repository to your `$PATH`:

    cmake -B build -G Ninja -DCMAKE_INSTALL_PREFIX=$HOME/.local
    ninja -C build/ install

Once this is done, and the firmware is loaded and connected to ESHET, you can
run:

    idf.py ota_flash

to build and flash the project. The esp32 will reboot into the new code, and
should reconnect. If all is well, run:

    idf.py ota_mark_valid

to mark this firmware as valid. Until this is done, if the device is restarted,
the previous firmware will be loaded. Otherwise, run:

    idf.py ota_mark_invalid

to mark the current firmware as invalid and reboot to the previous one. This
should work even if the current firmware has been marked as valid, if there is
another valid firmware to roll back to.

Finally, there's a command to just restart the device:

    idf.py ota_restart

## License

```
Copyright 2023 Thomas Nixon

This program is free software: you can redistribute it and/or modify it under
the terms of version 3 of the GNU General Public License as published by the
Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

See LICENSE.
```
