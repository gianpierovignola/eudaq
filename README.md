EUDAQ version 2
=====

[![Build Status](https://travis-ci.org/eudaq/eudaq.svg?branch=master)](https://travis-ci.org/eudaq/eudaq)
[![Build status](https://ci.appveyor.com/api/projects/status/n3tq45kkupyvjihg/branch/master?svg=true)](https://ci.appveyor.com/project/eudaq/eudaq/branch/master)



EUDAQ is a Generic Multi-platform Data Acquisition Framework.
Version 2 comes with more flexible cross connectivity between components, multiple data collectors, and a cleaner seperation between core functionalities and user modules. 

### License
This program is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

### Documentation and context

Please check out the online documentation at 
- http://eudaq.github.io/ or 
- the manual, see below
- (https://telescopes.desy.de/EUDAQ or)

### Sources of documentation within the framework

The user's manual is provided as LaTeX source files in the repository;
to generate the pdf on Linux/OSX, make sure that you have installed ImageMagick, then follow these steps:
```
cd build
cmake -D EUDAQ_BUILD_MANUAL=ON ..
make pdf
```
The manual can then be found in ```./doc/manual/EUDAQUserManual.pdf```.

To generate the doxygen documentation, make sure that you have installed doxygen and run
```
make doxygen
```
in the ```build``` directory after CMake. The resulting HTML files are stored in ```../doc/doxygen/html```.


## Prerequisites

### For the core Library, Executables and GUI
EUDAQ requires a C++11 compliant compiler and a Qt version 5 or higher to build GUIs. 
We recommend a gcc version 4.9 or later.
ROOT 6 is required for the Online Monitor GUI.

### For user modules

- user/eudet: Cactus/Ipbus Software for AIDA TLU and ZestSC1+Tlufirmware+Libusb for EUDET TLU
- user/timepix3: Spidr

## Compiling and installation

cmake will configure the installation and prepare the makefiles. It searches for all the required files. 
It has to be executed in a ```build``` folder, however the relevant CMakeLists.txt is in the main level, thus, the command is ```cmake ..```. 
If cmake is successful, EUDAQ can be installed. 
Variables set are cached in CMakeCache.txt and will again be taken into account at the next cmake run.

cmake has several options (```cmake -D OPTION=ON/OFF ..```) to activate or deactivate programs which will be built, here printed with their default value:  
- ```EUDAQ_BUILD_EXECUTABLE=ON```
- ```EUDAQ_BUILD_GUI=ON```
- ```EUDAQ_BUILD_MANUAL=OFF```
- ```EUDAQ_BUILD_PYTHON=OFF```
- ```EUDAQ_BUILD_STDEVENT_MONITOR=ON```
- ```EUDAQ_EXTRA_BUILD_NREADER=OFF```
- ```EUDAQ_LIBRARY_BUILD_LCIO=ON```
- ```EUDAQ_MODULE_BUILD_STD=ON```
- ```USER_CALICE_BUILD=OFF```
- ```USER_EUDET_BUILD=ON```
- ```USER_EUDET_BUILD_NI=ON```
- ```USER_EXAMPLE_BUILD=ON```
- ```USER_EXPERIMENTAL_BUILD=ON```
- ```USER_ITKSTRIP_BUILD=OFF```
- ```USER_STCONTROL_BUILD=ON```
- ```USER_TIMEPIX3_BUILD=OFF```

If cmake is not successful and complains about something is missing, it is recommended to clean the ```build``` folder by ```rm -rf *``` before a new try.
If problems occur during installation, please have a look in the issues, if a similiar problem already occured. If not, feel free to create a new ticket: https://github.com/eudaq/eudaq/issues

### Quick installation for UNIX


```
git clone -b master https://github.com/eudaq/eudaq.git
mkdir -p eudaq/build
cd eudaq/build
cmake ..
make install
```




### Notes for Windows

#### Visual Studio for compiling (MSVC)

- The recommended windows compiler is MSVC (Microsoft Visual C++) like Visual Studio 2013 and later
- Download Visual Studio Express Desktop (e.g. 2013 Version): http://www.microsoft.com/en-us/download/details.aspx?id=40787

#### Compiling using cmake syntax

Start the Visual Studio "Developer Command Prompt" from the Start Menu entries for Visual Studio (Tools subfolder) which opens a cmd.exe session with the necessary environment variables already set. 
If your Qt installation path has not been added to the global %PATH% variable, you need to execute the "qtenv2.bat" batch file in the Qt folder, e.g. and replace "5.1.1" with the version string of your Qt installation:
```
C:\Qt\Qt5.1.1\5.1.1\msvc2013\bin\qtenv2.bat
```
Go to the EUDAQ folder and configure, as describe above:
```
cd c:\[...]\eudaq\build
cmake ..
```
This generates the VS project files. Installing into eudaq\bin by:
```
cmake --build . --target install --config Release
```
### Notes for OS X

- Compiler: Clang (at least version 3.1)
- Install Qt5 or later, e.g. by using MacPorts (http://www.macports.org/): sudo port install qt5-mac-devel


## Execution

In UNIX:
```
cd ../user/example/misc
./example_startrun.sh
```
The startrun script assembles the new command line syntax: Core executables are started by loading a specific module with the name option ```-n``` assigning a unique tag by the option ```-t```:
```
cd bin
./euRun -n Ex0RunControl -a tcp://44000 &
sleep 1
./euLog -a tcp://44001&
sleep 1
./euCliMonitor -n Ex0Monitor -t my_mon  -a tcp://45001 &
./euCliCollector -n Ex0TgDataCollector -t my_dc -a tcp://45002 &
./euCliProducer -n Ex0Producer -t my_pd0 &
./euCliProducer -n Ex0Producer -t my_pd1 &
```

Play around with Init, Configure, Start, Stop, Re-Start or Re-Configure and Start or Reset and Re-Init and repeat -- or Terminate. 

For Initialising and Configuring you have to Load to set the path to the the ini or conf file:
- ../eudaq/user/example/misc/Ex0.ini
- ../eudaq/user/example/misc/Ex0.conf

A description for operating the EUDET-type beam telescopes is under construction:
https://telescopes.desy.de/User_manual
