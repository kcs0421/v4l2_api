#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/highgui/highgui.hpp>
#include <fstream>

#include "capturev4l2.hpp"

#define BUF 2

using namespace cv;
using namespace std;

static int xioctl(int fd, int request, void *arg)
{
    int r;

    do {
        r = ioctl (fd, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

CaptureV4L2::CaptureV4L2(){ 
    if(open_camera())
        return;
}

int CaptureV4L2::print_caps()
{
    struct v4l2_capability caps = {};
    if (-1 == xioctl(fd_, VIDIOC_QUERYCAP, &caps))
    {
        perror("Querying Capabilities");
        return 1;
    }

    printf( "Driver Caps:\n"
            "  Name: \"%s\"\n"
            "  Driver: \"%s\"\n",
            // "  Bus: \"%s\"\n"
            // "  Version: %d.%d\n"
            // "  Capabilities: %08x\n",
            caps.card,
            caps.driver);
            // caps.bus_info,
            // (caps.version>>16)&&0xff,
            // (caps.version>>24)&&0xff,
            // caps.capabilities);

    return 0;
}

int CaptureV4L2::set_pix_fmt()
{
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 1920;
    fmt.fmt.pix.height = 1080;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    
    if (-1 == xioctl(fd_, VIDIOC_S_FMT, &fmt))
    {
        perror("Setting Pixel Format");
        return 1;
    }

    char fourcc[5] = {0};

    strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
    printf( "Selected Camera Mode:\n"
            "  Width: %d\n"
            "  Height: %d\n"
            "  PixFmt: %s\n",
            // "  Field: %d\n",
            fmt.fmt.pix.width,
            fmt.fmt.pix.height,
            fourcc);
            // fmt.fmt.pix.field);

    return 0;
}

int CaptureV4L2::init_mmap()
{
    struct v4l2_requestbuffers req = {0};
    req.count = BUF;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
 
    if (-1 == xioctl(fd_, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        return 1;
    }
 
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd_, VIDIOC_QUERYBUF, &buf))
    {
        perror("Querying Buffer");
        return 1;
    }
    buffer_.index = buf.index;
    buffer_.length = buf.length;
    buffer_.start = static_cast<unsigned char *>(mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buf.m.offset));

    if (MAP_FAILED == buffer_.start) {
      printf("Failed mapping device memory");
      return 1;
    }
    printf("Length(file size): %d\n",
        // "Address: %p\n",
        buf.length);
        // buffer_.start);
 
    return 0;
}

int CaptureV4L2::capture_image()
{
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd_, VIDIOC_QBUF, &buf))
    {
        perror("Queue Buffer");
        return 1;
    }
 
    if(-1 == xioctl(fd_, VIDIOC_STREAMON, &buf.type))
    {
        perror("Start Capture");
        return 1;
    }
 
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_, &fds);
    struct timeval tv = {0};
    tv.tv_sec = 2;
    int r = select(fd_+1, &fds, NULL, NULL, &tv);
    if(-1 == r)
    {
        perror("Waiting for Frame");
        return 1;
    }
 
    if(-1 == xioctl(fd_, VIDIOC_DQBUF, &buf))
    {
        perror("Retrieving Frame");
        return 1;
    }
    printf("Image successfully captured\n");
 
    return 0;
}

int CaptureV4L2::save_img()
{
    FILE * pFile = fopen("image.raw", "wb");
    if(pFile==NULL){
        perror("ERROR: Cannot open output file");
        return 1;
    } 
    fwrite((const int *)buffer_.start , sizeof(char), buffer_.length, pFile);
    fclose(pFile);
    printf("Raw Image successfully saved with name image.raw\n");

    return 0;
}
int CaptureV4L2::save_jpeg()
{
    // //파일스트림: 파일을 읽고 쓸 수 있도록 지원하는 클래스
    // //raw 파일을 파일스트림 fs에 바이너리 형태로 저장
    // ifstream fs("image.raw", ios::binary);
    // //이미지데이터를 저장해놓을 버퍼를 생성(opencv mat 함수의 형태와 호환되도록 vector로 생성)
    // vector<char> buffer(buffer_.length);
    // //read로 파일스트림 fs에 저장된 이미지데이터를 버퍼에 불러옴
    // fs.read(buffer.data(), buffer_.length);
    // //버퍼의 이미지데이터를 mat 형식으로 변경
    // Mat uyvy2mat(Size(1920, 1080), CV_8UC2, buffer.data());
    // //mat 형식의 이미지데이터를 BGR로 변경
    // Mat uyvy2bgr;
    // cvtColor(uyvy2mat, uyvy2bgr, COLOR_YUV2BGR_UYVY);
    // imwrite("image3.jpeg", uyvy2bgr);

    //open
    FILE * pFile = fopen("image.raw", "rb");
    vector<unsigned char> buffer(buffer_.length);
    fread(buffer.data(), sizeof(unsigned char), buffer_.length, pFile);
    fclose(pFile);
    Mat uyvy2mat(Size(1920, 1080), CV_8UC2, buffer.data());
    Mat uyvy2bgr;
    cvtColor(uyvy2mat, uyvy2bgr, COLOR_YUV2BGR_UYVY);
    imwrite("image3.jpeg", uyvy2bgr);

    printf("jpeg saved\n");

    return 0;
}

int CaptureV4L2::open_camera()
{
    fd_ = open("/dev/video0", O_RDWR);
    if (fd_ == -1)
    {
        perror("Opening video device");
        //return 1;
    }
    else {
        printf("Successfully open device\n");
    }

    return 0;
}

int CaptureV4L2::stop_stream()
{
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(-1 == xioctl(fd_, VIDIOC_STREAMOFF, &buf.type))
    {
        perror("Stop Capture");
        return 1;
    }

    return 0;
}

void CaptureV4L2::close_camera()
{
    close(fd_);
}

void CaptureV4L2::run(){
    if(print_caps())
        return;
    if(set_pix_fmt())
        return;
    if(init_mmap())
        return;
    if(capture_image())
        return;
    if(save_img())
        return;
    if(save_jpeg())
        return;
    if(stop_stream())
        return;
    close_camera();
}

int main(int argc, char** argv)
{
    CaptureV4L2 capture_v4l2;

    capture_v4l2.run();

    return 0;
}
