# saltf

The location of a libraries folder should be placed relative to the project
directory structure as follows:

"<user_location>/Projects/saltf"

"<user_location>/Libraries/"

## Dependencies

### Windows

- FFTW3

Precompiled DLL files for FFTW can be found [here](http://www.fftw.org/install/windows.html)

fftw-3.3.5-dll64.zip should be unzipped and extracted to the 'Libraries' folder (shown above)

To install on Windows it is necessary to run the required commands to create import libraries.
Please see the link above for instructions on how to do that.

- SOFA

Source for the libsofa library can be found [here](https://github.com/sofacoustics/API_Cpp/)

The libsofa directory should be copied to the 'Libraries' folder (shown above)

This library can then be compiled using Visual Studio

- WDL

Source for a mirror of the WDL library can be found [here](https://github.com/justinfrankel/WDL/tree/master/WDL)

The WDL directory should be copied to the 'Libraries' folder (shown above)

- Eigen

Source for the Eigen library can be found [here](http://eigen.tuxfamily.org/index.php?title=Main_Page)

The Eigen directory should be copied to the 'Libraries' folder (shown above)
