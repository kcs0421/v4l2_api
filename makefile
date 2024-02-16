#!/bin/bash
CC = gcc
CFLAGS = -W -Wall
SRCS = v4l2.c
TARGET = test
$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(OBJECTS) $(TARGET) core
