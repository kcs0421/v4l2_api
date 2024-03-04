#!/bin/bash
CC = g++
CXXFLAGS = -Wall
SRCS = capturev4l2.cpp
TARGET = test
$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio

clean:
	rm -f $(OBJECTS) $(TARGET)
