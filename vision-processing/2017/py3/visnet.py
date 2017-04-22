# visnet.py -- Vision processing served over the network.
# Communicates to the robot using PyNetworkTables.
# Written for the 2017 FRC season (Steamworks) by Sebastian Mobo.
#
# Dependencies: opencv-python, numpy, pynetworktables

import numpy as np
import math
import cv2
import sys
import time
from networktables import NetworkTables
import viscore

# Configuration constants:
smoothing_constant = 0.25   # Low-pass filter constant: lower values = slower-changing but more stable values
robot_ip = "10.50.02.2"     # Static IP: don't change unless you're certain, and don't use the mDNS name
table_name = "vision"       # Table name to push values to: must match with robot code
cam_no = 0                  # Camera ID to open: usually 0 unless you have multiple cameras

# State variables, don't touch
filteredDist = 0
filteredOffset = 0
cam = cv2.VideoCapture(cam_no)

if not cam.isOpened():
    print("Error: could not open camera {}".format(cam_no))
    sys.exit(-1)

try:
    # Init NetworkTables:
    NetworkTables.initialize(server=robot_ip)
    vt = NetworkTables.getTable(table_name)

    vt.putBoolean("connected", True)

    # Processing loop:
    while True:
        ret, frame = cam.read()
        contours = viscore.preprocess(frame)
        tgt1, tgt2 = viscore.select_contours(contours)

        if (tgt1 is not None) and (tgt2 is not None):
            # Calculate distance to targets:
            #print(frame.shape)
            dist1 = getTargetDistance(tgt1[0], frame.shape[1], frame.shape[0])
            dist2 = getTargetDistance(tgt2[0], frame.shape[1], frame.shape[0])
            avgDist = (dist1+dist2)/2.0

            # Get target offsets:
            offset1 = getTargetOffset(tgt1[0], frame.shape[1], dist1)
            offset2 = getTargetOffset(tgt2[0], frame.shape[1], dist2)
            avgOffset = (offset1+offset2)/2.0

            filteredDist = filteredDist + (smoothing_constant * (avgDist - filteredDist))
            filteredOffset = filteredOffset + (smoothing_constant * (avgOffset - filteredOffset))

            vt.putBoolean("valid", True)
            vt.putNumber("distance", filteredDist)
            vt.putNumber("offset", filteredOffset)
        else:
            vt.putBoolean("valid", False)

        time.sleep(1/60) # note that time.sleep() uses units of seconds
except Exception as e:
    raise
finally:
    cv2.destroyAllWindows()
    cam.release()
