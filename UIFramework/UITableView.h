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

#ifndef __UITABLEVIEW_H__
#define __UITABLEVIEW_H__

#include "UIFramework/UIView.h"


#define DEFAULT_CACHE_SIZE 10


class UITableView : public UIView
{
public:
    UITableView(SharedPointer<UIView::Array>& table, uint32_t cacheSize = DEFAULT_CACHE_SIZE);
    ~UITableView();

    void scrollPx(int32_t speed);
    void updateTable();

    void scrollPxForward(uint32_t speed);
    void scrollPxBackward(uint32_t speed);

    void setCenter(uint32_t index);
    uint32_t getFirstIndex();
    uint32_t getMiddleIndex();
    uint32_t getLastIndex();

    uint32_t getFirstOverflow();

    void setPixels(int32_t pixels);
    int32_t getPixels();

    // UIView
    virtual uint32_t fillFrameBuffer(SharedPointer<FrameBuffer>& buffer, int16_t xOffset, int16_t yOffset);
    virtual void prefetch(int16_t xOffset, int16_t yOffset);
    void setWakeupCallback(FunctionPointer& wakeup);

protected:
    SharedPointer<UIView::Array> table;

private:
    void insertCell(SharedPointer<UIView>& cell, uint32_t index);
    SharedPointer<UIView>& getCellAtCacheIndex(uint32_t index);
    void prefetch(uint32_t index, minar::callback_handle_t* handle);

    uint32_t topRow;
    uint32_t topCellOverflow;

    uint32_t cacheSize;
    uint32_t* lookUpTable;
    SharedPointer<UIView>* cache;
    SharedPointer<UIView> cacheMiss;

    minar::callback_handle_t cacheTopCallbackHandle;
    minar::callback_handle_t cacheBottomCallbackHandle;

    int32_t outstandingScrollPx;
};

#endif // __UITABLEVIEW_H__

