# Spatial Audio Listening Test Environment (SALTE) - Audio Renderer

Perceptual evaluation of spatial audio plays a significant role in the development of future content production frameworks, for example in the assessment of spatial audio recording techniques and reproduction schemes, binaural rendering algorithms, Head-Related Transfer Function (HRTF) datasets, virtual soundscapes and room acoustics. SALTE is a listening test software which will significantly aid future research and development of spatial audio systems. The tool consists of a dedicated audio rendering engine and virtual reality interface for conducting spatial audio listening experiments. The renderer can be used for controlled playback of Ambisonic scenes (up to 7th order) over headphones using head tracking as well as custom HRTFs contained within separate WAV files or SOFA files.

## Dependencies

The location of a libraries folder should be placed relative to the project
directory structure as follows:

'(user_location)/(user_repository_directory)/SALTE-audio-renderer'

'(user_location)/Libraries/'

### Windows
- SOFA

Source for the libsofa library can be found [here](https://github.com/sofacoustics/API_Cpp/). The libsofa directory should be copied to the "Libraries" folder (shown above). This library can then be compiled using Visual Studio. It is also required to modify the PATH environment variable to point at the ".dll" location for "netcdf.dll".

- WDL

Source for a mirror of the WDL library can be found [here](https://github.com/justinfrankel/WDL/tree/master/WDL). The WDL directory should be copied to the 'Libraries' folder (shown above).

- ASIO SDK

To build and run the application as ASIO compliant software the Steinberg license agreement must be agreed to and signed,
and then the ASIO SDK should be download from [here](https://www.steinberg.net/en/company/developers.html) and its path set in the Projucer settings.

- Visual Studio Mtd something setting.

### MacOS
- SOFA

Source for the libsofa library can be found [here](https://github.com/sofacoustics/API_Cpp/). The libsofa directory should be copied to the "Libraries" folder (shown above). This library can then be compiled using Xcode.

- WDL

Source for a mirror of the WDL library can be found [here](https://github.com/justinfrankel/WDL/tree/master/WDL). The WDL directory should be copied to the 'Libraries' folder (shown above).

- Xcode Mtd something setting.
