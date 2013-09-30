# Cordova-cef #
This project is a [Cordova/Phongap](http://cordova.apache.org "Cordova") framework implementation based on the [Chromium Embedded Framework (CEF)](https://code.google.com/p/chromiumembedded "Chromium Embedded Framework (CEF)"). The future plan is to complete the the following implementations:

- Windows Desktop for Windows 7 or later (not Windows 8 Store APP)
- Windows 8 Store APP
- Mac OSX
- Linux

## Windows-Desktop for Windows 7 or later (not Windows 8 Store APP) ##
- Download [CEF3 binaries for Windows(32 Bit)](http://www.magpcss.net/cef_downloads/index.php "CEF3 binaries for Windows")
- Copy contents of CEF3 binaries package into `<ROOT>/3rdparty/cef3/windows` (tested with 3.1453.1255)
- Download the [Boost Libraries](http://www.boost.org/users/download/ "Boost Libraries")
- Copy the contents of the `boost_x_y_z` folder into `<ROOT>/3rdparty/boost`
- Open `<ROOT>/projects/windows-desktop/windows-desktop.sln` with Visual Studio 2012
- Update the Platform Toolset of the `libcef_dll_wrapper` project in Visual Studio to `Visual Studio 2012 (v110)` 
- Build Debug or Release
- After building the output can be foun in `<ROOT>/projects/windows-desktop/Debug` or `Release`



