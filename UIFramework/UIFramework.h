/* mbed
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __UIFRAMEWORK_H__
#define __UIFRAMEWORK_H__

#include "mbed-drivers/mbed.h"

#include "uif-framebuffer/FrameBuffer.h"

#include "uif-matrixlcd/MatrixLCD.h"
#include "UIFramework/UIView.h"


class UIFramework
{
public:
    /*  Framework for updating screen buffers based on UIView base object
        and transfer the result to the LCD screen.
    */
    UIFramework(uif::MatrixLCD& screen,
                SharedPointer<UIView>& baseView,
                uint32_t frameLimit = 0);

    /*  Request screen update.
    */
    void wakeupTask(void);

    /*  Set/get frame rate limit.
    */
    void setFrameLimit(uint32_t limit);
    uint32_t getFrameLimit(void) const;

private:
    void renderViewToCurrentBuffer(void);
    void copyBufferToScreenDone(void);
    void updateScreen(void);

private:
    uif::MatrixLCD& screen;
    SharedPointer<UIView> baseView;

    SharedPointer<FrameBuffer> canvas;

    uint32_t callInterval;
    bool screenBusy;
    bool screenUpdateTaskNotPosted;
    bool renderBufferTaskNotPosted;
    bool frameReady;

    uint32_t frameLimit;
};


#endif // __UIFRAMEWORK_H__
