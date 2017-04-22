# lib5002: Various reusable bits.

## Swerve Drive
The included code is meant to be dropped into an existing robot project.


## Vision Processing 2017
Python vision processing code.

### pip Dependencies:
 * opencv-python
 * numpy
 * pynetworktables
### Operation:
Run visnet.py on the Jetson, and use the Jetson.java subsystem on the robot;
NetworkTables handles communication between the two processes.

## Vision Processing 2016
C++ vision processing code. Intended to be run on both X86-64 and hard-float ARM.
Compile on a Linux system with G++ >= 4.9.
Supports the x86-64 and gnueabihf toolchains.

### Make Targets for Libraries:
 * lib5002-vis.so: shared library containing vision processing code.
 * lib5002-net.so: shared library containing networking code.
 * lib5002-stream.so: shared library contaning code to support network video streams.

### Make Targets for Testing Programs:
 * ballproc: Ball processing test.
 * goalproc: Goal processing test.
 * goalproc-basic: Basic goal processing test (no realtime visual output, just console)
 * nettest: Networking test (echo server).
 * disctest: Network discovery protocol test.

Also: the 'outdirs' target will automatically create directories to put output / build files in.

Define ARCH=ARM to enable builds for ARMHF.
Binaries and library files will be output in the ./bin/ subfolder.
