# Vision Processing 2017
Python vision processing code.

## pip Dependencies
 * `opencv-python`
 * `numpy`
 * `pynetworktables`

## VisCore (Vision Processing Core)
This module contains common code for performing the following functions:
 * Camera frame preprocessing
 * Contour filtering and selection
 * Target distance and lateral offset calculation

## VisNet (Networked Vision)
The VisNet client uses VisCore to find and report vision processing information over NetworkTables.

To be specific, it connects to the robot NetworkTables server (at a preconfigured static IP address),
opens the `vision` table, and regularly pushes data over it with the following keys:
 * `connected` - will be set to true if the VisNet client is connected
 * `valid` - will be set to true if the targets could be detected and if valid distance/offset data could be obtained
 * `distance` - the filtered distance to the targets in inches
 * `offset` - the filtered lateral offset from the target center in inches

## Jetson Subsystem
The Jetson Subsystem interfaces with the robot NetworkTables server to receive values
pushed from the Jetson-side VisNet instance.

The subsystem exposes all of the above keys via its member methods.

## VisProc (Vision Processing Debugging)
This program is intended for debugging VisCore, and displays:
 * Processed / thresholded frames
 * Detected candidate contours
 * Calculated distances / offsets
 * Calculated FOV angles (given a configurable constant distance to the targets)
