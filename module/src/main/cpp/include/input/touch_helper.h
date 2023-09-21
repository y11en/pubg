//
// Created by tao.wan on 2023/9/20.
//

#ifndef RIRU_MODULETEMPLATE_MASTER_TOUCH_HELPER_H
#define RIRU_MODULETEMPLATE_MASTER_TOUCH_HELPER_H


bool nativeInit(char* mDevName);

void touchSwip(long startX, long startY, long endX, long endY, long finger, long duration);

bool exit();


#endif //RIRU_MODULETEMPLATE_MASTER_TOUCH_HELPER_H
