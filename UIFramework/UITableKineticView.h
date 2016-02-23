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


#ifndef __UITABLEKINETICVIEW_H__
#define __UITABLEKINETICVIEW_H__

#include "UIFramework/UITableView.h"


class UITableKineticView : public UITableView
{
public:
    UITableKineticView(SharedPointer<UIView::Array>& table,
                       uint32_t width,
                       uint32_t height,
                       int32_t  offset);

    void sliderPressed();

    void sliderChangedWithSpeed(int32_t speedPx);
    void sliderReleasedWithSpeed(int32_t speedPx);

    int32_t getFirstOffset();
    int32_t getLastOffset();

    // UIView
    virtual uint32_t fillFrameBuffer(SharedPointer<FrameBuffer>& buffer,
                                     int16_t xOffset,
                                     int16_t yOffset);

protected:
    uint32_t friction;

private:
    bool findMagnetism();

private:
    int32_t magnetism;
    int32_t coasting;
    bool sliderNotPressed;

    /* offsets are always negative because the screen coordinate system is opposite the table coordinate system */
    int32_t firstOffset;
    int32_t lastOffset;
    int32_t globalOffset;
};

#endif // __UITABLEKINETICVIEW_H__
