//
// Created by tao.wan on 2023/9/20.
//

#include "touch_helper.h"
#include "Logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <sys/limits.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <errno.h>
#include <zconf.h>
#include <set>

using namespace std;
/**
 * 驱动位置目录
 */
static char *devDir = "/dev/input";
static struct pollfd *ufds = NULL;
static char *device_names = NULL;
static int BTN_TOUCH_DOWN = 0x00000001;
/**
 * 驱动文件的描述符
 */
int devFd = -1;

static int device_name_equals(int deviceFd, const char *device_name) {
    char name[80];
    name[sizeof(name) - 1] = '\0';
    if (ioctl(deviceFd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
        fprintf(stderr, "could not get device name for %d, %s, %s\n", deviceFd, device_name,
                strerror(errno));
        return -1;
    }
    //printf("device name: %s\n", name);
    if (strcmp(name, device_name) == 0) {
        return 1;
    }
    return 0;
}

int open_device_by_name_fd(const char *dirname, const char *device_name) {
    ufds = static_cast<pollfd *>(calloc(1, sizeof(ufds[0])));
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if (dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while ((de = readdir(dir))) {
        if (de->d_name[0] == '.' &&
            (de->d_name[1] == '\0' ||
             (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        int fd = open(devname, O_RDWR);
        if (fd < 0) {
            fprintf(stderr, "could not open %s, %s\n", devname, strerror(errno));
            return -1;
        }
        //printf("open device: %s\n", devname);
        if (device_name_equals(fd, device_name) == 1) {
            device_names = devname;
            return fd;
        } else {
            close(fd);
        }
    }
    closedir(dir);
    return -1;
}

bool nativeInit(char *mDevName) {
    devFd = open_device_by_name_fd(devDir, mDevName);
    if (devFd == -1) {
        LOGI("设备驱动初始化 失败");
        return false;
    } else {
        LOGI("设备驱动初始化 成功");
        return true;
    }
}

bool sendOperationCmd(long type, long code, long value) {
    LOGI("向设备驱动发送信息 type: %ld code: %ld value: %ld", type, code, value);
    struct input_event event;

    memset(&event, 0, sizeof(event));

    event.type = type;
    event.code = code;
    event.value = value;

    if (devFd != -1) {
        ssize_t writeSize = write(devFd, &event, sizeof(event));
        if (writeSize == sizeof(event)) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void touchDown(long x, long y, long finger) {
    sendOperationCmd(EV_ABS, ABS_MT_SLOT, finger);

    sendOperationCmd(EV_ABS, ABS_MT_TRACKING_ID, finger);

    sendOperationCmd(EV_KEY, BTN_TOUCH, BTN_TOUCH_DOWN);

    sendOperationCmd(EV_ABS, ABS_MT_POSITION_X, x);

    sendOperationCmd(EV_ABS, ABS_MT_POSITION_Y, y);

    sendOperationCmd(EV_SYN, 0, 0);

}

void touchUp(long finger) {
    sendOperationCmd(EV_ABS, ABS_MT_SLOT, finger);

    sendOperationCmd(EV_ABS, ABS_MT_TRACKING_ID, -1);

    sendOperationCmd(EV_SYN, 0, 0);
}

void touchMove(long x, long y, long finger) {

    sendOperationCmd(EV_ABS, ABS_MT_SLOT, finger);
    sendOperationCmd(EV_KEY, BTN_TOUCH, BTN_TOUCH_DOWN);
    sendOperationCmd(EV_ABS, ABS_MT_POSITION_X, x);
    sendOperationCmd(EV_ABS, ABS_MT_POSITION_Y, y);
    sendOperationCmd(EV_SYN, 0, 0);
}


void touchSwip(long startX, long startY, long endX, long endY, long finger, long duration) {
    touchDown(startX, startY, finger);

    double xiDistance = abs(startX - endX);

    double yiDistance = abs(startY - endY);

    double xDelta = xiDistance / duration;

    double yDelta = yiDistance / duration;

    for (long i = 0; i < duration; i++) {
        touchMove((long) (xDelta * i + startX), (long) (yDelta * i + startY), finger);
        sleep(1);
    }
    touchUp(finger);
}

bool exit() {

    LOGI("设备驱动初 退出");

    int result = close(devFd);
    if (result == 0) {

        return true;
    } else {
        return false;
    }
}
