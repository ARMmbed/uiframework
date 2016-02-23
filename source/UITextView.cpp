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

#include "UIFramework/UITextView.h"

#include <cstdlib>

UITextView::UITextView(const char* _text, const struct FontData* _font)
    :   UIView(),
        text(_text),
        font(_font),
        textString(),
        mallocBuffer(NULL),
        cacheImage(NULL)
{
    // call helper function to initialise object
    constructor();
}

UITextView::UITextView(std::string& _string, const struct FontData* _font)
    :   UIView(),
        font(_font),
        textString(_string),
        mallocBuffer(NULL),
        cacheImage(NULL)
{
    // get pointer to locally cached string
    text = textString.c_str();

    // call helper function to initialise object
    constructor();
}

void UITextView::constructor()
{
    if ((text != NULL) && (font != NULL))
    {
        struct FontMetrics metric = fontMetricsForStr(font, text);

        textTop = font->base - metric.y_offset; // number of pixels from baseline to tallest character

        contentWidth = metric.width;
        contentHeight = metric.height;

        // set size in parent
        UIView::width = metric.width;
        UIView::height = metric.height;

        // set dimentions in cache but do not allocate
        cacheBuffer.buf  = NULL;
        cacheBuffer.mask = (uint8_t*)Comp_Fill_Ones;
        cacheBuffer.bit_offset = 0;
        cacheBuffer.stride_bytes = (contentWidth + 7) / 8; // ceil(contentWidth / 8)
        cacheBuffer.width_bits   = contentWidth;
        cacheBuffer.height_strides = contentHeight;
    }
    else
    {
        text = NULL;
        font = NULL;
    }
}

UITextView::~UITextView()
{
    free(mallocBuffer);
    delete cacheImage;
}

uint32_t UITextView::fillFrameBuffer(SharedPointer<FrameBuffer>& canvas, int16_t xOffset, int16_t yOffset)
{
    prefetch(0, 0);

    /* Copy text to canvas */
    if (cacheImage != NULL)
    {
        cacheImage->setInverse(inverse);
        cacheImage->setHorizontalAlignment(align);
        cacheImage->setVerticalAlignment(valign);
        cacheImage->setWidth(width);
        cacheImage->setHeight(height);

        cacheImage->fillFrameBuffer(canvas, xOffset, yOffset);
    }

    return ULONG_MAX;
}

void UITextView::prefetch(int16_t xOffset, int16_t yOffset)
{
    (void) xOffset;
    (void) yOffset;

    if ((mallocBuffer == NULL) && (text != NULL))
    {
        uint32_t numBytes = cacheBuffer.stride_bytes * cacheBuffer.height_strides;

        mallocBuffer = (uint8_t*) calloc(numBytes, sizeof(uint8_t));

        if (mallocBuffer != NULL)
        {
            cacheBuffer.buf = mallocBuffer;

            fontRenderStr(cacheBuffer, font, textTop, text, 1);

            cacheBuffer.mask = cacheBuffer.buf;
            cacheBuffer.buf = (uint8_t*)Comp_Fill_Zeros;

            cacheImage = new UIImageView(&cacheBuffer);
        }
    }
}
