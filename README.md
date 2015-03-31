# cordova-cef #
This project is a [Cordova/Phongap](http://cordova.apache.org "Cordova") framework implementation based on the [Chromium Embedded Framework (CEF)](https://code.google.com/p/chromiumembedded "Chromium Embedded Framework (CEF)"). 

[cordova-cef](https://github.com/hsimpson/cordova-cef "cordova-cef") **is not yet production ready** and is not fully integrated into the [Cordova](http://cordova.apache.org "Cordova") eco system, which means it is not usable with the cordova-cli.

The future plan is to complete the following implementations:

- Windows Desktop for Windows 7 or later (not Windows 8/8.1 Store-App)
- Mac OSX
- Linux
- Windows 8.1 Store-App


## Windows-Desktop for Windows 7 or later (not Windows 8/8.1 Store-App) ##
- Download and install [CMake](http://www.cmake.org/) (minimum and tested version: 2.8.12.2)
- Download [CEF3 binaries for Windows 32bit](http://cefbuilds.com "CEF3 binaries for Windows") (mostly the trunk is used because this project is under high development tested with CEF 3.2339.1251)
- Copy contents of CEF3 binaries package into `<ROOT>/3rdparty/cef3`
- Run the cmake command in the `<ROOT>/3rdparty/cef3` directory:
    cmake -G "Visual Studio 12"
- Open `<ROOT>/projects/windows-desktop/windows-desktop.sln` with Visual Studio 2013
- Build Debug or Release
- After building the output can be found in `<ROOT>/dist/windows-desktop/Debug` or `Release`


## Mac OSX ##
not yet implemented


## Linux ##
not yet implemented


## Windows 8.1 Store-App
not yet implemented
