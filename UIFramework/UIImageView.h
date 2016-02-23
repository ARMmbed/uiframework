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

#ifndef __UIIMAGEVIEW_H__
#define __UIIMAGEVIEW_H__

#include "UIFramework/UIView.h"

class UIImageView : public UIView
{
public:
    UIImageView(const struct CompBuf* image);

    // from UIView
    virtual ~UIImageView();
    virtual uint32_t fillFrameBuffer(SharedPointer<FrameBuffer>& canvas,
                                     int16_t xOffset,
                                     int16_t yOffset);

private:
    const struct CompBuf* image;
    uint32_t contentWidth;
    uint32_t contentHeight;
};

#endif // __UIIMAGEVIEW_H__
