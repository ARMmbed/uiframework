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

#ifndef __UITEXTVIEW_H__
#define __UITEXTVIEW_H__

#include "UIFramework/UIView.h"
#include "UIFramework/UIImageView.h"

#include "uif-tools-1bit/font.h"
#include "uif-tools-1bit/fonts/fonts.h"

#include <string>

class UITextView : public UIView
{
public:
    UITextView(std::string& text, const struct FontData* font);
    UITextView(const char* text, const struct FontData* font);

    // from UIView
    virtual ~UITextView();
    virtual uint32_t fillFrameBuffer(SharedPointer<FrameBuffer>& buffer,
                                     int16_t xOffset,
                                     int16_t yOffset);

    virtual void prefetch(int16_t xOffset, int16_t yOffset);

private:
    void constructor();

private:
    const char* text;
    const struct FontData* font;

    uint16_t textTop;
    uint16_t contentWidth;
    uint16_t contentHeight;

    std::string textString;
    uint8_t* mallocBuffer;
    struct CompBuf cacheBuffer;
    UIImageView* cacheImage;
};

#endif // __UITEXTVIEW_H__
