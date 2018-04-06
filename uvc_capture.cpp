/*** USB camera test application on Beagle board using v4l2 APIs
  It will capture the frames and write into screen
 Author:Srinivas
 Date:4/5/2018 ***/


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <fstream>
#include <string>

using namespace std;

int main()
{
    
    int fd0; 
    fd0 = open("/dev/video1",O_RDWR);
    if(fd0 < 0){
        perror("Failed to open device, OPEN");
        return 1;
    }

   
    v4l2_capability capability;
    if(ioctl(fd0, VIDIOC_CROPCAP, &capability) < 0){
         perror("VIDIOC_CROPCAP");
        return 1;
    }
    // We can Choose frame width and height 1280x720,1024x1024,640x640
    v4l2_format imageFormat;
    imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imageFormat.fmt.pix.width = 1280;
    imageFormat.fmt.pix.height = 720;
    imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    imageFormat.fmt.pix.field = V4L2_FIELD_NONE;
    
    if(ioctl(fd0, VIDIOC_S_PARM, &imageFormat) < 0){
        perror("VIDIOC_S_PARM");
        return 1;
    }
   
    v4l2_requestbuffers requestBuffer = {0};
    requestBuffer.count = 1; 
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    requestBuffer.memory = V4L2_MEMORY_MMAP;

    if(ioctl(fd0, VIDIOC_REQBUFS, &requestBuffer) < 0){
        perror("VIDIOC_REQBUFS");
        return 1;
    }
    
    v4l2_buffer queryBuffer = {0};
    queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queryBuffer.memory = V4L2_MEMORY_MMAP;
    queryBuffer.index = 0;
    if(ioctl(fd0, VIDIOC_QUERYBUF, &queryBuffer) < 0){
        perror("VIDIOC_QUERYBUF");
        return 1;
    }
    
    char* buffer = (char*)mmap(NULL, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                        fd0, queryBuffer.m.offset);
    memset(buffer, 0, queryBuffer.length);

    
    v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;

    
    int type = bufferinfo.type;
    if(ioctl(fd0, VIDIOC_STREAMON, &type) < 0){
        perror("VIDIOC_STREAMON");
        return 1;
    }

    
    if(ioctl(fd0, VIDIOC_QBUF, &bufferinfo) < 0){
        perror("VIDIOC_QBUF");
        return 1;
    }

    
    if(ioctl(fd0, VIDIOC_DQBUF, &bufferinfo) < 0){
        perror("VIDIOC_DQBUF");
        return 1;
    }

    cout << "Buffer has: " << (double)bufferinfo.bytesused / 1024
            << " KBytes of data" << endl; 
   
    ofstream uvcFile;
    uvcFile.open("uvcmeta_output.jpeg", ios::binary| ios::app);  
    int bufPos = 0, filememBlockSize = 0;  
                                        
    int bufferSize = bufferinfo.bytesused; 
                                                    
    char* outFileMemBlock = NULL;  
    int num_itr = 0; 
    while(bufferSize > 0) {
        bufPos += filememBlockSize; 

        filememBlockSize = 1024;   
        outFileMemBlock = new char[sizeof(char) * filememBlockSize];

        
        memcpy(outFileMemBlock, buffer+bufPos, filememBlockSize);
        uvcFile.write(outFileMemBlock,filememBlockSize);

        
        if(filememBlockSize > bufferSize)
            filememBlockSize = bufferSize;

       
        bufferSize -= filememBlockSize;

        
        cout << num_itr++ << " Remaining bytes size: "<< bufferSize << endl;
    }

	 uvcFile.close();
    // streaming off
    if(ioctl(fd0, VIDIOC_STREAMOFF, &type) < 0){
        perror("VIDIOC_STREAMOFF");
        return 1;
    }

    close(fd0);
    return 0;
}

