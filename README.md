# FRC 2016 Native C++ Code
Team 5002 (Dragon Robotics)'s code. Intended to be run on both X86-64 and hard-float ARM.
Compile on a Linux system.

# Make Targets:
 * goalproc: Goal processing test environment.
 * ballproc: Boulder detection test environment.
 * trajectory: Trajectory calculation sub-library.
 * nettest: Networking test (echo server). Builds for ARM only.
'goalproc' and 'ballproc' can be built for ARM by adding '-arm' to the target name.
