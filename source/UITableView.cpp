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


#include "UIFramework/UITableView.h"

#include "mbed-drivers/mbed.h"


#if 0
#include <stdio.h>
#define UIF_PRINTF(...) { printf(__VA_ARGS__); }
#else
#define UIF_PRINTF(...)
#endif

UITableView::UITableView(SharedPointer<UIView::Array>& _table, uint32_t _cacheSize)
    :   UIView(),
        table(_table),
        topRow(0),
        topCellOverflow(0),
        cacheSize(_cacheSize),
        lookUpTable(NULL),
        cache(NULL),
        cacheTopCallbackHandle(NULL),
        cacheBottomCallbackHandle(NULL),
        outstandingScrollPx(0)
{
    cache = new SharedPointer<UIView>[cacheSize];
    lookUpTable = new uint32_t[cacheSize];

    /* initialize cache */
    for (uint32_t idx = 0; idx < cacheSize; idx++)
    {
        lookUpTable[idx] = 0xFFFFFFFF;
    }
}

UITableView::~UITableView()
{
    delete[] cache;
    delete[] lookUpTable;

    // Cancel any callbacks that might have been scheduled but not executed
    if (cacheTopCallbackHandle)
    {
        minar::Scheduler::cancelCallback(cacheTopCallbackHandle);
    }

    if (cacheBottomCallbackHandle)
    {
        minar::Scheduler::cancelCallback(cacheBottomCallbackHandle);
    }
}

SharedPointer<UIView>& UITableView::getCellAtCacheIndex(uint32_t index)
{
    uint32_t tableIndex = index % cacheSize;

    if (lookUpTable[tableIndex] == index)
    {
        return cache[tableIndex];
    }
    else
    {
        return cacheMiss;
    }
}

void UITableView::insertCell(SharedPointer<UIView>& cell, uint32_t index)
{
    if (cell->isCacheable())
    {
        uint32_t tableIndex = index % cacheSize;

        lookUpTable[tableIndex] = index;

        cache[tableIndex] = cell;
    }
}

void UITableView::prefetch(uint32_t index, minar::callback_handle_t* handle)
{
    SharedPointer<UIView> cell = getCellAtCacheIndex(index);

    if ((cell == NULL) || (!cell->isValid()))
    {
        UIF_PRINTF("UITableView: prefetch: %lu\r\n", index);

        // get cell at index
        cell = table->viewAtIndex(index);

        // propagate wakeup callback
        cell->setWakeupCallback(wakeupCallback);

        // propagate color inversion
        cell->setInverse(inverse);

        // prefetch cell content
        cell->prefetch(0, 0);

        // insert cell in cache
        insertCell(cell, index);
    }

    // reset handle
    if (handle)
    {
        *handle = NULL;
    }
}

void UITableView::scrollPx(int32_t pixels)
{
    outstandingScrollPx += pixels;
}

void UITableView::updateTable()
{
    if (outstandingScrollPx > 0)
    {
        scrollPxBackward(outstandingScrollPx);
    }
    else if (outstandingScrollPx < 0)
    {
        scrollPxForward(-outstandingScrollPx);
    }

    outstandingScrollPx = 0;
}

// Update internal view
// Updates variables topRow and topCellOverflow
void UITableView::scrollPxForward(uint32_t pixels)
{
    uint32_t tableSize = table->getSize();

    if (tableSize > 1)
    {
        uint32_t totalHeight = 0;

        // find the last visible row and the new top row
        bool searchForTop = true;
        uint32_t newTopRow = topRow;
        uint32_t newTopCellOverflow = 0;

        uint32_t row = topRow;
        for ( ; (row < tableSize) && (totalHeight < (height + topCellOverflow + pixels)); row++)
        {
            uint32_t cellHeight = table->heightAtIndex(row);

            totalHeight += cellHeight;

            if (searchForTop)
            {
                if ((totalHeight - topCellOverflow) > pixels)
                {
                    searchForTop = false;

                    newTopRow = row;
                    newTopCellOverflow = cellHeight - (totalHeight - topCellOverflow - pixels);
                }
            }
        }

        // we reached the end of the array without filling the screen
        if ( (row == tableSize) && (totalHeight < (height + topCellOverflow + pixels)) )
        {
            // reverse search
            uint32_t reverseRow = tableSize - 1;
            uint32_t reverseHeight = 0;
            for ( ; (reverseRow > 0) && (reverseHeight < height); reverseRow-- )
            {
                uint32_t cellHeight = table->heightAtIndex(reverseRow);

                reverseHeight += cellHeight;
            }

            if (reverseHeight >= height)
            {
                topRow = reverseRow + 1;
                topCellOverflow = reverseHeight - height;
            }
            // we reached the top without filling the screen
            // i.e. there are not enough rows to fill it
            else
            {
                topRow = 0;
                topCellOverflow = 0;
            }
        }
        // filled the screen
        else // if (totalHeight >= (visibleScreen + topCellOverflow + pixels))
        {
            topRow = newTopRow;
            topCellOverflow = newTopCellOverflow;
        }
    }
}

void UITableView::scrollPxBackward(uint32_t pixels)
{
    /*  The scroll amount is smaller than the topRow overflow.
        Top row is unchanged.
    */
    if (topCellOverflow > pixels)
    {
        topCellOverflow -= pixels;
    }
    else
    {
        /*  Find new top row and offset by traversing array in reverse.
            topCellOverflow is the height of topRow, so start with topRow - 1.
         */
        int32_t reverseRow = topRow - 1;
        uint32_t totalHeight = 0;

        for ( ; (reverseRow >= 0) && ((totalHeight + topCellOverflow) < pixels); reverseRow--)
        {
            uint32_t cellHeight = table->heightAtIndex(reverseRow);

            totalHeight += cellHeight;
        }

        if ((totalHeight + topCellOverflow) >= pixels)
        {
            topRow = reverseRow + 1;
            topCellOverflow = totalHeight + topCellOverflow - pixels;
        }
        else
        {
            topRow = 0;
            topCellOverflow = 0;
        }
    }
}

void UITableView::setPixels(int32_t pixels)
{
    topRow = 0;
    topCellOverflow = 0;
    outstandingScrollPx = 0;

    if (pixels > 0)
    {
        scrollPxBackward(pixels);
    }
    else
    {
        scrollPxForward(-pixels);
    }
}

int32_t UITableView::getPixels()
{
    uint32_t heightSum = 0;

    for (uint32_t row = 0; row < topRow; row++)
    {
        heightSum += table->heightAtIndex(row);
    }

    return -(heightSum + topCellOverflow);
}

void UITableView::setCenter(uint32_t index)
{
    uint32_t tableSize = table->getSize();

    if (index < tableSize)
    {
        uint32_t heightSum = 0;

        for (uint32_t row = 0; row < index; row++)
        {
            heightSum += table->heightAtIndex(row);
        }

        heightSum += table->heightAtIndex(index) / 2;

        setPixels(-(heightSum - (height / 2)));
    }
}

uint32_t UITableView::getFirstOverflow()
{
    return topCellOverflow;
}

uint32_t UITableView::getFirstIndex()
{
    return topRow;
}

uint32_t UITableView::getMiddleIndex()
{
    uint32_t tableSize = table->getSize();
    uint32_t heightSum = table->heightAtIndex(topRow) - topCellOverflow;

    uint32_t row = topRow + 1;

    for (; (row < tableSize) && (heightSum < (height / 2)); row++)
    {
        heightSum += table->heightAtIndex(row);
    }

    return row - 1;
}

uint32_t UITableView::getLastIndex()
{
  uint32_t tableSize = table->getSize();
  uint32_t heightSum = table->heightAtIndex(topRow) - topCellOverflow;

  uint32_t row = topRow + 1;

  for (; (row < tableSize) && (heightSum < height); row++)
  {
      heightSum += table->heightAtIndex(row);
  }

  return row - 1;
}

/*  UIView */
uint32_t UITableView::fillFrameBuffer(SharedPointer<FrameBuffer>& canvas, int16_t xOffset, int16_t yOffset)
{
    int32_t heightSum = 0;
    int32_t maxHeight = height;
    uint32_t tableSize = table->getSize();

    uint32_t callInterval = ULONG_MAX;
    SharedPointer<FrameBuffer> subCanvas;

    SharedPointer<UIView> cell;

    /*  If canvas is NULL it means we are pre-fetching only and not actually blitting.
        If the Width/Height is not set, use the whole canvas.
    */
    if (canvas.get() != NULL)
    {
        if (width == 0)
        {
            width = canvas->getWidth();
        }

        if (height == 0)
        {
            height = canvas->getHeight();
        }

        maxHeight = canvas->getHeight();

        // fill background according to the inverse parameter
        if (inverse)
        {
            canvas->drawRectangle(0, width, 0, height, 0);
        }
        else
        {
            canvas->drawRectangle(0, width, 0, height, 1);
        }
    }

    // update table view based on scrolling
    updateTable();

    /*  Special case the top row. Necessary to do proper over-the-top drawing.
        Get cell from the cache if it exists and is still valid.
        Otherwise get it from the table-object and put it in the cache.
    */
    cell = getCellAtCacheIndex(topRow);

    if ((cell == NULL) || (!cell->isValid()))
    {
        UIF_PRINTF("UITableView: miss: %lu\r\n", topRow);

        // get cell at index
        cell = table->viewAtIndex(topRow);

        // propagate wakeup callback
        cell->setWakeupCallback(wakeupCallback);

        // propagate color inversion
        cell->setInverse(inverse);

        // insert cell in cache
        insertCell(cell, topRow);

        UIF_PRINTF("UITableView: cell: %p\r\n", cell.get());
    }


    /*  cellHeight is the height of the actual cell, according to the table-object.
        heightSum is the running sum of the table height as it is being created
        and has the initial value: yOffset, if it is negative. Only cells with
        a positive heightSum will be drawn.

        The top row can overflow when the table is scrolled, the number of pixels
        running over the top is stored in topCellOverflow.
    */
    int32_t cellHeight = table->heightAtIndex(topRow);
    int32_t yBase = (yOffset < 0) ? yOffset : 0;
    heightSum = yBase + cellHeight - topCellOverflow;

    if (heightSum > 0)
    {
        /* if canvas is NULL it means we are pre-fetching and not actually rendering */
        if (canvas.get() != NULL)
        {
            if (cell->getWidth() == 0)
            {
                cell->setWidth(width);
            }

            if (cell->getHeight() == 0)
            {
                cell->setHeight(cellHeight);
            }

            /*  Create a sub partition for each cell in the table to specify the area each
                cell is allowed to modify.
            */
            subCanvas = canvas->getFrameBuffer(0, 0, width, heightSum);

            /*  If the top cell doesn't fit the canvas, draw the bottom part of the cell by adjustsing
                the yOffset parameter. Otherwise use the difference in heightSum and cellHeight as yOffset.
                Note the canvas coordinate system is opposite the table's.
            */
            if (heightSum > canvas->getHeight())
            {
                callInterval = cell->fillFrameBuffer(subCanvas, xOffset, (canvas->getHeight() - heightSum));
            }
            else
            {
                callInterval = cell->fillFrameBuffer(subCanvas, xOffset, (heightSum - cellHeight));
            }
        }
        else
        {
            cell->prefetch(0, 0);
        }
    }

    /* loop through the rest of the rows, stop when CompBuf is full or end of table-> */
    uint32_t row = topRow + 1;

    for (; (row < tableSize) && (heightSum < maxHeight); row++)
    {
        cell = getCellAtCacheIndex(row);

        if ((cell == NULL) || (!cell->isValid()))
        {
            UIF_PRINTF("UITableView: miss: %lu\r\n", row);

            // get cell at index
            cell = table->viewAtIndex(row);

            // propagate wakeup callback
            cell->setWakeupCallback(wakeupCallback);

            // propagate color inversion
            cell->setInverse(inverse);

            // insert cell in cache
            insertCell(cell, row);

            UIF_PRINTF("UITableView: cell: %p\r\n", cell.get());
        }

        cellHeight = table->heightAtIndex(row);

        int32_t tempHeight = heightSum + cellHeight;

        if (tempHeight > 0)
        {
            /* if canvas is NULL it means we are pre-fetching and not actually rendering */
            if (canvas.get() != NULL)
            {
                if (cell->getWidth() == 0)
                {
                    cell->setWidth(width);
                }

                if (cell->getHeight() == 0)
                {
                    cell->setHeight(cellHeight);
                }

                subCanvas = canvas->getFrameBuffer(0, heightSum, width, cellHeight);

                if (tempHeight < cellHeight)
                {
                    uint32_t interval = cell->fillFrameBuffer(subCanvas, xOffset, (tempHeight - cellHeight));

                    callInterval = (callInterval < interval) ? callInterval : interval;
                }
                else
                {
                    uint32_t interval = cell->fillFrameBuffer(subCanvas, xOffset, 0);

                    callInterval = (callInterval < interval) ? callInterval : interval;
                }
            }
            else
            {
                cell->prefetch(0, 0);
            }
        }

        heightSum += cellHeight;
    }

    /* schedule offscreen cells to be pre-cached */
    /* cache the cell before the ones already shown if it is not already in cache */
    if (topRow > 0)
    {
        FunctionPointer2<void, uint32_t, minar::callback_handle_t*> fetchRow(this, &UITableView::prefetch);

        cacheTopCallbackHandle = minar::Scheduler::postCallback(fetchRow.bind(topRow, &cacheTopCallbackHandle))
                                    .getHandle();
    }

    /* cache the cell after the ones already shown if it is not already in cache */
    if (row < tableSize)
    {
        FunctionPointer2<void, uint32_t, minar::callback_handle_t*> fetchRow(this, &UITableView::prefetch);

        cacheBottomCallbackHandle = minar::Scheduler::postCallback(fetchRow.bind(row, &cacheBottomCallbackHandle))
                                        .getHandle();
    }

    return callInterval;
}

void UITableView::prefetch(int16_t xOffset, int16_t yOffset)
{
    SharedPointer<FrameBuffer> null;

    fillFrameBuffer(null, xOffset, yOffset);
}

void UITableView::setWakeupCallback(FunctionPointer& callback)
{
    UIF_PRINTF("UITableView: set wakeup %p\r\n", callback.get_function());

    /* store reference for new objects pushed on the stack. */
    wakeupCallback = callback;

    for (uint32_t idx = 0; idx < cacheSize; idx++)
    {
        if (cache[idx] != NULL)
        {
            cache[idx]->setWakeupCallback(wakeupCallback);
        }
    }
}

