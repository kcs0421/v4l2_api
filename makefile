#!/bin/bash
CC = g++
CXXFLAGS = -Wall
SRCS = capturev4l2.cpp
OPENCV = -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio
TARGET = test
$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(OPENCV)

clean:
	rm -f $(OBJECTS) $(TARGET)
