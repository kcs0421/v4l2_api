#!/usr/bin/python3

import cv2

mydevice = "/dev/video0"
myapi = cv2.CAP_V4L2
myvideo = cv2.VideoCapture(mydevice, myapi)
video_name = "output"

if myvideo.isOpened():
    cv2.namedWindow(video_name, cv2.WINDOW_NORMAL)
    while 1:
        retval, img = myvideo.read()
        cv2.imshow(video_name, img)
        k = cv2.waitKey(10)
        if k & 0xFF == ord('q'):
            cv2.imwrite("./capture.png", img)
            break
    cv2.destroyAllWindows
    myvideo.release()
