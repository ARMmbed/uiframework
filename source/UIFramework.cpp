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

#include "UIFramework/UIFramework.h"

#include <sys/time.h>

#if (YOTTA_CFG_HARDWARE_WRD_SWO_PRESENT \
  && YOTTA_CFG_HARDWARE_WRD_SWO_ENABLED)
#include "swo/swo.h"
#define UIF_PRINTF(...) { swoprintf(__VA_ARGS__); }
#else
#define UIF_PRINTF(...)
#endif

UIFramework::UIFramework(uif::MatrixLCD& _screen,
                         SharedPointer<UIView>& _baseView,
                         uint32_t _frameLimit)
    :   screen(_screen),
        baseView(_baseView),
        callInterval(ULONG_MAX),
        screenBusy(false),
        screenUpdateTaskNotPosted(true),
        renderBufferTaskNotPosted(true),
        frameReady(false),
        frameLimit(_frameLimit)
{
    /* Post once to get the screen started. This call starts the initial screen drawing.
    */
    minar::Scheduler::postCallback(this, &UIFramework::wakeupTask);

    /* Register wakeup callback with UIView root node. */
    FunctionPointer wakeup(this, &UIFramework::wakeupTask);

    baseView->setWakeupCallback(wakeup);
}

void UIFramework::renderViewToCurrentBuffer()
{
    renderBufferTaskNotPosted = true;

    /* calculate frame to screen time */
    uint32_t renderStart = 0;
    uint32_t frameRate = 0;
    struct timeval now;

    /* start timer */
    gettimeofday(&now, NULL);
    renderStart = (now.tv_sec * 1000) + (now.tv_usec / 1000);

    /* Grab buffer not used by screen */
/*
    struct CompBuf canvas = {
      .buf  = screen.getBuffer(),
      .mask = (uint8_t*)Comp_Fill_Ones,
      .bit_offset = 0,
      .stride_bytes = stride,
      .width_bits   = width,
      .height_strides = height
    };
*/
    canvas = screen.getFrameBuffer();

    /* fill canvas. return value is the requested refresh rate in millisecond. */
    callInterval = baseView->fillFrameBuffer(canvas, 0, 0);

    /* end timer */
    gettimeofday(&now, NULL);
    frameRate = (now.tv_sec * 1000) + (now.tv_usec / 1000) - renderStart;

    /* adjust call interval to account for rendering time if not ULONG_MAX */
    if (callInterval != ULONG_MAX)
    {
        callInterval = (callInterval < frameRate) ? 0 : callInterval - frameRate;

        /* adjust call interval to limit frame rate */
        if (callInterval < frameLimit)
        {
            callInterval = frameLimit;
        }
    }

    UIF_PRINTF("Framework: render: %lu %u\r\n", frameRate, callInterval);

    /*  If a frame is already being sent, wait. Otherwise initiate screen
        transfer.
    */
    if (screenBusy)
    {
        frameReady = true;
    }
    else
    {
        screenBusy = true;

        /*  If animation is in progress schedule the next screen calculation to
            start when transfer starts.
        */
        if (callInterval == 0)
        {
            renderBufferTaskNotPosted = false;

            FunctionPointer onStart(this, &UIFramework::renderViewToCurrentBuffer);
            FunctionPointer onFinish(this, &UIFramework::copyBufferToScreenDone);

            screen.sendFrameBuffer(canvas, onStart, onFinish);
        }
        else
        {
            FunctionPointer onFinish(this, &UIFramework::copyBufferToScreenDone);

            screen.sendFrameBuffer(canvas, 0, onFinish);
        }
    }
}

/* Call back block for when the screen transfer is complete */
void UIFramework::copyBufferToScreenDone()
{
    UIF_PRINTF("Framework: screen: done\r\n");

    if (frameReady)
    {
        frameReady = false;

        if (callInterval == 0)
        {
            renderBufferTaskNotPosted = false;

            FunctionPointer onStart(this, &UIFramework::renderViewToCurrentBuffer);
            FunctionPointer onFinish(this, &UIFramework::copyBufferToScreenDone);

            screen.sendFrameBuffer(canvas, onStart, onFinish);
        }
        else
        {
            FunctionPointer onFinish(this, &UIFramework::copyBufferToScreenDone);

            screen.sendFrameBuffer(canvas, 0, onFinish);
        }
    }
    else
    {
        screenBusy = false;

        /* use the callback interval to set sleep time */
        /* Note: callback delay cannot be greater than time wrap-around. */
        if (callInterval != ULONG_MAX)
        {
            UIF_PRINTF("Framework: callback: %lu\r\n", callInterval);

            minar::Scheduler::postCallback(this, &UIFramework::wakeupTask)
                .tolerance(minar::milliseconds(0))
                .delay(minar::milliseconds(callInterval));
        }
    }
}

/*  Redraw screen. If the previous rounds of draws are still running, do nothing.
*/
void UIFramework::updateScreen()
{
    /*  Only one render task can be posted at a time and it should only
        be posted if there isn't already a new frame ready in the buffer.
    */
    if (renderBufferTaskNotPosted && !frameReady)
    {
        renderBufferTaskNotPosted = false;

        minar::Scheduler::postCallback(this, &UIFramework::renderViewToCurrentBuffer)
            .tolerance(minar::milliseconds(0));
    }
    else
    {
        callInterval = frameLimit;
    }

    screenUpdateTaskNotPosted = true;
}

/*  This task guards the screenUpdateTask from being called multiple times.
*/
void UIFramework::wakeupTask()
{
    UIF_PRINTF("Framework: wakeup requested\r\n");

    if (screenUpdateTaskNotPosted)
    {
        UIF_PRINTF("Framework: update\r\n");

        screenUpdateTaskNotPosted = false;
        minar::Scheduler::postCallback(this, &UIFramework::updateScreen)
            .tolerance(minar::milliseconds(0));
    }
}

/*  Set/get frame rate limit.
*/
void UIFramework::setFrameLimit(uint32_t _frameLimit)
{
    frameLimit = _frameLimit;
}

uint32_t UIFramework::getFrameLimit(void) const
{
    return frameLimit;
}
