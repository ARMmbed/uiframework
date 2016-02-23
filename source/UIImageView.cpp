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

#include "UIFramework/UIImageView.h"


UIImageView::UIImageView(const struct CompBuf* _image)
    :   UIView(),
        image(_image)
{
    if (image)
    {
        contentWidth = image->width_bits;
        contentHeight = image->height_strides;

        UIView::width = image->width_bits;
        UIView::height = image->height_strides;
    }
    else
    {
        contentWidth = 0;
        contentHeight = 0;

        UIView::width = 0;
        UIView::height = 0;
    }
}

UIImageView::~UIImageView()
{
}

uint32_t UIImageView::fillFrameBuffer(SharedPointer<FrameBuffer>& canvas, int16_t xOffset, int16_t yOffset)
{
    /* use canvas dimensions if none has been pre-set */
    if (UIView::width == 0)
    {
        UIView::width = canvas->getWidth();
    }

    if (UIView::height == 0)
    {
        UIView::height = canvas->getHeight();
    }

    int32_t xbase = 0;
    int32_t ybase = 0;

    /* horizontal alignment */
    if (align == UIView::ALIGN_CENTER)
    {
        xbase = (UIView::width - contentWidth) / 2;
    }
    else if (align == UIView::ALIGN_RIGHT)
    {
        xbase = UIView::width - contentWidth;
    }

    /* vertical alignment */
    if (valign == UIView::VALIGN_MIDDLE)
    {
        ybase = (UIView::height - contentHeight) / 2;
    }
    else if (valign == UIView::VALIGN_BOTTOM)
    {
        ybase = (UIView::height - contentHeight);
    }

    if (inverse)
    {
        if (image)
        {
            const struct CompBuf inverseImage = {
                (uint8_t*)Comp_Fill_Ones,
                image->mask,
                image->bit_offset,
                image->stride_bytes,
                image->width_bits,
                image->height_strides
            };

            canvas->drawImage(inverseImage, xbase + xOffset, ybase + yOffset, 0);
        }
    }
    else
    {
        if (image)
        {
            canvas->drawImage(*image, xbase + xOffset, ybase + yOffset, 0);
        }
    }

    return ULONG_MAX;
}
