import numpy as np
import math
import cv2
import sys
import viscore

smoothing_constant = 0.25
filteredDist = 0
filteredOffset = 0
filteredFOVVert = 0
filteredFOVHoriz = 0

cam = cv2.VideoCapture(1)

if not cam.isOpened():
    print("Error: could not open camera 1")
    sys.exit(-1)

try:
    while True:
        ret, frame = cam.read()
        contours, filtered = viscore.preprocess(frame)
        tgt1, tgt2, passedContours, failedContours = viscore.select_contours(contours)

        # Draw debug info:
        filtered = cv2.cvtColor(cv2.COLOR_GRAY2BGR)
        filtered_frame = cv2.bitwise_or(frame, filtered)

        # Fill in target contours with blue:
        out = cv2.addWeighted(frame, 0.5, filtered_frame, 0.5)
        out = cv2.drawContours(out, [tgt1[0], tgt2[0]], -1, (255, 0, 0), -1)

        # Draw rectangles around candidate contours:
        for ct in passedContours:
            pts = get_minimal_rect_pts(ct)
            cv2.drawContours(out, [pts], 0, (0, 255, 0), 4)

        # Calculate distance to targets:
        #print(frame.shape)
        dist1 = viscore.get_target_distance(tgt1[0], frame.shape[1], frame.shape[0])
        dist2 = viscore.get_target_distance(tgt2[0], frame.shape[1], frame.shape[0])
        print(str.format("d1: {:.3f} inches", dist1))
        print(str.format("d2: {:.3f} inches", dist2))

        # Get FOV angles:
        angles1 = viscore.get_fov_angles(tgt1[0], frame.shape[1], frame.shape[0], 72.0)
        angles2 = viscore.get_fov_angles(tgt2[0], frame.shape[1], frame.shape[0], 72.0)

        avgFOVHoriz = (angles1[0]+angles2[0])/2.0
        avgFOVVert = (angles1[1]+angles2[1])/2.0
        avgDist = (dist1+dist2)/2.0

        # Get target offsets:
        offset1 = viscore.get_target_offset(tgt1[0], frame.shape[1], dist1)
        offset2 = viscore.get_target_offset(tgt2[0], frame.shape[1], dist2)
        avgOffset = (offset1+offset2)/2.0

        filteredFOVHoriz = filteredFOVHoriz + (smoothing_constant*(avgFOVHoriz-filteredFOVHoriz))
        filteredFOVVert = filteredFOVVert + (smoothing_constant*(avgFOVVert-filteredFOVVert))
        filteredDist = filteredDist + (smoothing_constant * (avgDist - filteredDist))
        filteredOffset = filteredOffset + (smoothing_constant * (avgOffset - filteredOffset))

        cv2.putText(out, str.format("Distance: {:.3g} inches", filteredDist), (25, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0))
        cv2.putText(out, str.format("Lateral Offset: {:.3g} inches", filteredOffset), (25, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0))
        cv2.putText(out, str.format("Approx. horiz. FOV: {:f} radians,", filteredFOVHoriz), (25, 75), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0))
        cv2.putText(out, str.format("Approx. vert. FOV: {:f} radians,", filteredFOVVert), (25, 100), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0))

        cv2.imshow("Output", out)

        if cv2.waitKey(int(1000/60)) == 27:
            break
except Exception as e:
    raise
finally:
    cv2.destroyAllWindows()
    cam.release()
