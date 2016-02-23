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

#include "UIFramework/UITableKineticView.h"


#if 0
#include <stdio.h>
#define UIF_PRINTF(...) { printf(__VA_ARGS__); }
#else
#define UIF_PRINTF(...)
#endif

UITableKineticView::UITableKineticView(SharedPointer<UIView::Array>& table,
                                       uint32_t width,
                                       uint32_t height,
                                       int32_t  offset)
    :   UITableView(table),
        friction(5),
        magnetism(0),
        coasting(0),
        sliderNotPressed(true),
        globalOffset(offset)
{
    UIView::width = width;
    UIView::height = height;

    /* read dimensions */
    UITableView::setCenter(table->getLastIndex());
    lastOffset = UITableView::getPixels() + globalOffset;

    UITableView::setCenter(table->getFirstIndex());
    firstOffset = UITableView::getPixels() + globalOffset;

    UITableView::setCenter(table->getDefaultIndex());
}

int32_t UITableKineticView::getFirstOffset()
{
    return firstOffset;
}

int32_t UITableKineticView::getLastOffset()
{
    return lastOffset;
}

bool UITableKineticView::findMagnetism()
{
    int32_t offset = UITableView::getPixels();

    if (offset < lastOffset)
    {
        magnetism = lastOffset - offset;
    }
    else if (offset > firstOffset)
    {
        magnetism = firstOffset - offset;
    }
    else
    {
        uint32_t firstIndex = UITableView::getFirstIndex();
        uint32_t firstOverflow = UITableView::getFirstOverflow();
        uint32_t firstHeight = table->heightAtIndex(firstIndex);

        int32_t heightSum = firstHeight - firstOverflow;

        uint32_t tableIndex = firstIndex + 1;
        uint32_t tableSize = table->getSize();

        for (; (tableIndex < tableSize) && (heightSum < (UIView::height / 2 + globalOffset)); tableIndex++)
        {
            heightSum += table->heightAtIndex(tableIndex);
        }

        uint32_t overflow = heightSum - (UIView::height / 2 + globalOffset);
        uint32_t cellHeight = table->heightAtIndex(tableIndex - 1);

        if (overflow > (cellHeight / 2))
        {
            magnetism = -(overflow - (cellHeight / 2) - (cellHeight % 2));
        }
        else if (overflow < (cellHeight / 2))
        {
            magnetism = (cellHeight / 2) - overflow;
        }
        else
        {
            magnetism = 0;
        }
    }

    return (magnetism != 0);
}

void UITableKineticView::sliderPressed()
{
    UIF_PRINTF("UTKV: pressed\r\n");

    magnetism = 0;
    sliderNotPressed = false;

    if (wakeupCallback)
    {
        wakeupCallback();
    }
}

void UITableKineticView::sliderChangedWithSpeed(int32_t speedPx)
{
    UIF_PRINTF("UTKV: changed\r\n");

    int32_t filteredSpeed = 0;

    if (speedPx > (int32_t) UIView::height)
    {
        filteredSpeed = UIView::height;
    }
    else if (speedPx < -((int32_t) UIView::height))
    {
        filteredSpeed = -UIView::height;
    }
    else
    {
        filteredSpeed = speedPx;
    }

    int32_t offset = UITableView::getPixels();

    /* larger means slower */
    int32_t scaledSpeed;

    if (offset < lastOffset)
    {
        scaledSpeed = filteredSpeed / 3;
    }
    else if (offset > firstOffset)
    {
        scaledSpeed = filteredSpeed / 3;
    }
    else
    {
        scaledSpeed = filteredSpeed;
    }

    UITableView::scrollPx(scaledSpeed);
}

void UITableKineticView::sliderReleasedWithSpeed(int32_t speedPx)
{
    UIF_PRINTF("UTKV: released\r\n");

    sliderNotPressed = true;

    int32_t filteredSpeed = 0;

    if (speedPx > (int32_t) UIView::height)
    {
        filteredSpeed = UIView::height;
    }
    else if (speedPx < -((int32_t) UIView::height))
    {
        filteredSpeed = -UIView::height;
    }
    else
    {
        filteredSpeed = speedPx;
    }

    if ((filteredSpeed > 10) || (filteredSpeed < -10))
    {
        coasting = filteredSpeed;
    }
    else
    {
        findMagnetism();
    }
}


/*  UIView */
uint32_t UITableKineticView::fillFrameBuffer(SharedPointer<FrameBuffer>& canvas, int16_t xOffset, int16_t yOffset)
{
    uint32_t callInterval = ULONG_MAX;

    if (sliderNotPressed && (xOffset == 0) && (yOffset == 0))
    {
        if (coasting != 0)
        {
            /* set friction based on whether we are inside our outside the table. */
            int32_t offset = UITableView::getPixels();

            if ((offset < lastOffset) || (offset > firstOffset))
            {
                coasting = coasting / 2;
            }

            UITableView::scrollPx(coasting);
            callInterval = 0;

            if (coasting > (int32_t) friction)
            {
                coasting -= friction;
            }
            else if (coasting < -((int32_t) friction))
            {
                coasting += friction;
            }
            else
            {
                coasting = 0;
            }
        }
        else if (magnetism != 0)
        {
            int32_t scroll = magnetism / 2;

            if (scroll == 0)
            {
                scroll = magnetism;
            }

            UITableView::scrollPx(scroll);
            magnetism -= scroll;
            callInterval = 0;
        }
        else
        {
            if (findMagnetism() != 0)
            {
                callInterval = 0;
            }
        }
    }

    uint32_t interval = UITableView::fillFrameBuffer(canvas, xOffset, yOffset);
    callInterval = (callInterval < interval) ? callInterval : interval;

    return (!sliderNotPressed) ? 0 : callInterval;
}
