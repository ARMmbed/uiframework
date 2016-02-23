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

#include "UIFramework/UIViewStack.h"


#if 0
#include <stdio.h>
#define UIF_PRINTF(...) { printf(__VA_ARGS__); }
#else
#define UIF_PRINTF(...)
#endif

UIViewStack::UIViewStack()
    :   UIView(),
        transitionTimeInMilliSeconds(250),
        scrollLeftToRight(false),
        scrollRightToLeft(false),
        scrollOffset(0),
        scrollStartTime(0),
        stack()
{
    UAllocTraits_t traits = {0};

    stack.init(5, 5, traits);
}

UIViewStack::~UIViewStack()
{
}

void UIViewStack::pushView(SharedPointer<UIView>& view)
{
    /* add callback for wakeups. */
    view->setWakeupCallback(wakeupCallback);

    /*  Put UIView at the end of array.
        Cycle mainCell to leftCell, new view to rightCell.
        If leftCell is not nil, start scrolling rightToLeft.
        Else, use UIView as main view.
        The recorded time is used for timing the scrolling transition time.
    */
    stack.push_back(view);

    leftCell = mainCell;
    mainCell = view;
    rightCell = view;

    if (leftCell)
    {
        scrollRightToLeft = true;
        scrollLeftToRight = false;
        scrollStartTime = UIView::getTimeInMilliseconds();

        leftCell->suspend();
    }

    rightCell->resume();
}

SharedPointer<UIView>& UIViewStack::popView()
{
    /*  Once in use, the stack can not be empty since fillCompBuf call goes through the stack.
        Only remove element if there are at least one left afterwards.
        Return the top element of the stack and not the discarded one, since that is the one
        we are interested in manipulating.
        The recorded time is used for timing the scrolling transition time.
    */
    unsigned stackSize = stack.get_num_elements();

    if (stackSize > 1)
    {
        rightCell = stack.at(stackSize - 1);
        stack.pop_back();

        leftCell = stack.at(stackSize - 2);
        mainCell = leftCell;

        rightCell->suspend();
        leftCell->resume();

        scrollLeftToRight = true;
        scrollRightToLeft = false;
        scrollStartTime = UIView::getTimeInMilliseconds();
    }

    return mainCell;
}

SharedPointer<UIView>& UIViewStack::resetView()
{
    /*  Remove all elements except the last one.
    */
    unsigned stackSize = stack.get_num_elements();

    if (stackSize > 1)
    {
        rightCell = stack.at(stackSize - 1);

        // remove all elements except last one
        while (stackSize > 1)
        {
            stack.pop_back();
            stackSize--;
        }

        leftCell = stack.at(0);
        mainCell = leftCell;

        // elements in stack are always suspended
        // only need to suspend and resume last and first element
        rightCell->suspend();
        leftCell->resume();

        scrollLeftToRight = true;
        scrollRightToLeft = false;
        scrollStartTime = UIView::getTimeInMilliseconds();
    }

    return mainCell;
}

uint32_t UIViewStack::getSize()
{
    return stack.get_num_elements();
}

uint32_t UIViewStack::getTransitionTime()
{
    return transitionTimeInMilliSeconds;
}

void UIViewStack::setTransitionTime(uint32_t timeInMilliseconds)
{
    transitionTimeInMilliSeconds = timeInMilliseconds;
}

SharedPointer<UIView::Action> UIViewStack::getAction()
{
    return mainCell->getAction();
}

/*  UIView */
uint32_t UIViewStack::fillFrameBuffer(SharedPointer<FrameBuffer>& canvas,
                                      int16_t xOffset,
                                      int16_t yOffset)
{
#warning xOffset and yOffset transpose in UIViewStack not implemented
    /*  FIXME: [UIViewStack fillCompBuf] does not use xOffset and yOffset to transpose itself. */
    (void) xOffset;
    (void) yOffset;

    uint32_t callInterval = ULONG_MAX;

    SharedPointer<FrameBuffer> left_canvas;
    SharedPointer<FrameBuffer> right_canvas;

    if (scrollRightToLeft)
    {
        /*  Calculate scrolling offset based on transitionTime and elapsed time since scrolling was initiated.
        */
        uint32_t now = UIView::getTimeInMilliseconds();
        uint32_t progress = now - scrollStartTime;
        scrollOffset = (UIView::width * progress) / transitionTimeInMilliSeconds;

        /*  The scrolling is over when the offset has been cycled through a complete perceived width.
        */
        if (scrollOffset < UIView::width)
        {
            left_canvas = canvas->getFrameBuffer(-scrollOffset,
                                                 0,
                                                 UIView::width,
                                                 UIView::height);

            leftCell->fillFrameBuffer(left_canvas, -scrollOffset, 0);

            right_canvas = canvas->getFrameBuffer(UIView::width - scrollOffset,
                                                  0,
                                                  UIView::width,
                                                  UIView::height);

            rightCell->fillFrameBuffer(right_canvas, 0, 0);

            callInterval = 0;
        }
        else
        {
            /*  Reset variables. Set mainCell since this is the one we are calling when not scrolling.
            */
            scrollRightToLeft = false;
            scrollOffset = 0;

            callInterval = rightCell->fillFrameBuffer(canvas, 0, 0);
        }
    }
    else if (scrollLeftToRight)
    {
        uint32_t now = UIView::getTimeInMilliseconds();
        uint32_t progress = now - scrollStartTime;
        scrollOffset = (UIView::width * progress) / transitionTimeInMilliSeconds;

        if (scrollOffset < UIView::width)
        {
            left_canvas = canvas->getFrameBuffer(scrollOffset - UIView::width,
                                                 0,
                                                 UIView::width,
                                                 UIView::height);

            leftCell->fillFrameBuffer(left_canvas, (scrollOffset - UIView::width), 0);

            right_canvas = canvas->getFrameBuffer(scrollOffset,
                                                  0,
                                                  UIView::width,
                                                  UIView::height);

            rightCell->fillFrameBuffer(right_canvas, 0, 0);

            callInterval = 0;
        }
        else
        {
            scrollLeftToRight = false;
            scrollOffset = 0;
            rightCell = SharedPointer<UIView>();

            callInterval = leftCell->fillFrameBuffer(canvas, 0, 0);
        }
    }
    else
    {
        /* no scrolling in prorgess, call main cell */
        callInterval = mainCell->fillFrameBuffer(canvas, 0, 0);
    }

    return callInterval;
}

void UIViewStack::prefetch(int16_t xOffset, int16_t yOffset)
{
    mainCell->prefetch(xOffset, yOffset);
}

void UIViewStack::setWakeupCallback(FunctionPointer& callback)
{
    UIF_PRINTF("UIViewStack: set wakeup %p\r\n", callback.get_function());

    /* store reference for new objects pushed on the stack. */
    wakeupCallback = callback;

    /* apply callback reference to objects already in stack. */
    unsigned stackSize = stack.get_num_elements();

    for (unsigned idx = 0; idx < stackSize; idx++)
    {
        stack.at(idx)->setWakeupCallback(callback);
    }
}

