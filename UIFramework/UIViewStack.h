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

#ifndef __UIVIEWSTACK_H__
#define __UIVIEWSTACK_H__


#include "UIFramework/UIView.h"

#include "core-util/Array.h"

class UIViewStack : public UIView
{
public:
    UIViewStack();
    ~UIViewStack();

    uint32_t getTransitionTime();
    void setTransitionTime(uint32_t timeInMilliseconds);

    void pushView(SharedPointer<UIView>& view);
    SharedPointer<UIView>& popView();
    SharedPointer<UIView>& resetView();

    uint32_t getSize();

    // UIView
    virtual uint32_t fillFrameBuffer(SharedPointer<FrameBuffer>& buffer, int16_t xOffset, int16_t yOffset);
    virtual void prefetch(int16_t xOffset, int16_t yOffset);
    virtual void setWakeupCallback(FunctionPointer& wakeup);

    virtual SharedPointer<UIView::Action> getAction();

private:
    uint32_t transitionTimeInMilliSeconds;

    bool scrollLeftToRight;
    bool scrollRightToLeft;
    uint32_t scrollOffset;
    uint32_t scrollStartTime;

    mbed::util::Array<SharedPointer<UIView> > stack;

    SharedPointer<UIView> mainCell;
    SharedPointer<UIView> leftCell;
    SharedPointer<UIView> rightCell;
};

#endif // __UIVIEWSTACK_H__
