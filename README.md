# SALTE Audio Renderer

The location of a libraries folder should be placed relative to the project
directory structure as follows:

"<user_location>/Projects/SALTE-audio-renderer"

"<user_location>/Libraries/"

## Dependencies

### Windows
- SOFA

Source for the libsofa library can be found [here](https://github.com/sofacoustics/API_Cpp/)

The libsofa directory should be copied to the 'Libraries' folder (shown above)

This library can then be compiled using Visual Studio

It is also required to modify the PATH environment variable to point at the .dll location for netcdf.dll

- WDL

Source for a mirror of the WDL library can be found [here](https://github.com/justinfrankel/WDL/tree/master/WDL)

The WDL directory should be copied to the 'Libraries' folder (shown above)

- ASIO SDK

To build and run the application as ASIO compliant software the Steinberg license agreement must be agreed to and signed,
and then the ASIO SDK should be download from [here](https://www.steinberg.net/en/company/developers.html)
