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

#ifndef __UIVIEW_H__
#define __UIVIEW_H__


#include "mbed-drivers/mbed.h"

#include "core-util/FunctionPointer.h"
#include "core-util/SharedPointer.h"

#include "uif-framebuffer/FrameBuffer.h"

#include <climits>
#include <string>

using namespace mbed::util;
using namespace uif;

class UIView
{
public:
    typedef enum {
      VALIGN_TOP,
      VALIGN_MIDDLE,
      VALIGN_BOTTOM
    } valign_t;

    typedef enum {
      ALIGN_LEFT,
      ALIGN_CENTER,
      ALIGN_RIGHT
    } align_t;

    class Array;

    /**
     * @brief Return type for when the default action is invoked on a cell.
     */
    class Action
    {
    public:
        typedef enum {
            None,
            Back,
            View,
            Array
        } type_t;

        /**
         * @brief Action constructor with type.
         *
         * @param type Action type.
         */
        Action(type_t type = None);

        /**
         * @brief Action constructor with UIView-object.
         *
         * @param view UIView-object to be returned.
         */
        Action(SharedPointer<UIView>& view);

        /**
         * @brief Action constructor with UIView::Array-object.
         *
         * @param array UIView::Array-object to be returned.
         */
        Action(SharedPointer<UIView::Array>& array);

        /**
         * @brief Get UIView-object if present.
         *
         * @return UIView-object.
         */
        SharedPointer<UIView> getView(void) const;

        /**
         * @brief Get UIView::Array if present.
         *
         * @return UIView::Array-object.
         */
        SharedPointer<UIView::Array> getArray(void) const;

        /**
         * @brief Get Action-object type.
         *
         * @return [None, Back, View, Array]
         */
        type_t getType(void) const;

    private:
        type_t type;
        SharedPointer<UIView> view;
        SharedPointer<UIView::Array> array;
    };

    /**
     * @brief Storage container for tables.
     */
    class Array
    {
    public:
        /**
         * @brief Optional destructor.
         */
        virtual ~Array(void) { };

        /**
         * @brief Get number of elements in the menu.
         *
         * @return Number of elements
         */
        virtual uint32_t getSize(void) const = 0;

        /**
         * @brief Get UIView object at the given index.
         *
         * @param index Cell to retrieve. Index must be between getFirstIndex
         *              and getLastIndex.
         * @return UIView-object wrapped inside a SharedPointer
         */
        virtual SharedPointer<UIView> viewAtIndex(uint32_t index) const = 0;

        /**
         * @brief Get pixel height of the cell at the given index.
         *
         * @param index Cell to get height of. Index must be between getFirstIndex
         *              and getLastIndex.
         * @return Height in number of pixels.
         */
        virtual uint32_t heightAtIndex(uint32_t index) const = 0;

        /**
         * @brief Get pixel width of the cell at the given index.
         *
         * @param index Cell to get width of. Index must be between getFirstIndex
         *              and getLastIndex.
         * @return Width in number of pixels.
         */
        virtual uint32_t widthAtIndex(uint32_t index) const = 0;

        /**
         * @brief Get the table's title.
         *
         * @return const char* to '/0'-terminated string. Can be NULL.
         */
        virtual const char* getTitle(void) const = 0;

        /**
         * @brief Get the lowest valid index for this table.
         *
         * @return Lowest valid index.
         */
        virtual uint32_t getFirstIndex(void) const { return 0; }

        /**
         * @brief Get the highest valid index for this table.
         *
         * @return Highest valid index.
         */
        virtual uint32_t getLastIndex(void) const { return 0; }

        /**
         * @brief Get the default index for this table.
         *
         * @return Default index.
         */
        virtual uint32_t getDefaultIndex(void) const { return 0; }

        /**
         * @brief Invoke action associated with the cell at the given index.
         *
         * @param index Cell to invoke action on.
         * @return UIView::Action-object wrapped in a SharedPointer. This object
         *         contains the result of the invoked action.
         */
        virtual SharedPointer<UIView::Action> actionAtIndex(uint32_t index)
        {
            (void) index;

            return SharedPointer<UIView::Action>(new UIView::Action());
        }
    };

    /**
     * @brief UIView destructor.
     */
    virtual ~UIView(void) { };

    /**
     * @brief Fill frame buffer with the content of the UIView-object.
     *
     * @param canvas FrameBuffer-object wrapped in a SharedPointer.
     * @param xOffset Number of pixels the camera has been translated along the
     *        horizontal axis.
     * @param yOffset Number of pixels the camera has been translated along the
     *        vertical axis.
     * @return The time in milliseconds to when the object wants to be called
     *         again. This is a lower-bound, meaning calling the function sooner
     *         will only result in the same data being filled into the frame
     *         buffer again.
     */
    virtual uint32_t fillFrameBuffer(SharedPointer<FrameBuffer>& buffer,
                                     int16_t xOffset,
                                     int16_t yOffset) = 0;

    /**
     * @brief Optional optimization call before calling fillFrameBuffer.
     * @details Object can compute and cache the relevant data for the given
     *          offsets. Useful for rendering strings internally before copying
     *          to frame buffer.
     *
     * @param xOffset Number of pixels the camera has been translated along the
     *        horizontal axis.
     * @param yOffset Number of pixels the camera has been translated along the
     *        vertical axis.
     */
    virtual void prefetch(int16_t xOffset, int16_t yOffset)
    {
        (void) xOffset;
        (void) yOffset;
    }

    /**
     * @brief Set callback function for waking up the UI framework,
     *        which requests a screen update.
     *
     * @param wakeup Callback function pointer.
     */
    virtual void setWakeupCallback(FunctionPointer& wakeup);

    /**
     * @brief Suspend activity in cell.
     * @details Function is called when UIView-object is out-of-view and/or
     *          when the object is put in the background.
     */
    virtual void suspend(void) { };

    /**
     * @brief Resume activity in cell.
     * @details Function is called when UIView-object is brought back into
     *          view.
     */
    virtual void resume(void) { };

    /**
     * @brief Activate action in object.
     *
     * @return UIView::Action-object wrapped in a SharedPointer.
     */
    virtual SharedPointer<UIView::Action> getAction(void);

    /**
     * @brief Get horizontal alignment.
     * @details Alignment determines how the object is positioned if the canvas
     *          is larger than the object.
     *
     * @return [ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT]
     */
    align_t getHorizontalAlignment(void) const;

    /**
     * @brief Get vertical alignment.
     * @details Alignment determines how the object is positioned if the canvas
     *          is larger than the object.
     *
     * @return [VALIGN_TOP, VALIGN_MIDDLE, VALIGN_BOTTOM]
     */
    valign_t getVerticalAlignment(void) const;

    /**
     * @brief Get object width in pixels.
     *
     * @return Object width in pixels.
     */
    uint16_t getWidth(void) const;

    /**
     * @brief Get object height in pixels.
     *
     * @return Object height in pixels.
     */
    uint16_t getHeight(void) const;

    /**
     * @brief Get inverse status.
     * @details Inverse flag is used for inverting the color of an object.
     *
     * @return Boolean status.
     */
    bool getInverse(void) const;

    /**
     * @brief Set horizontal alignment.
     * @details Alignment determines how the object is positioned if the canvas
     *          is larger than the object.
     *
     * @param align [ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT]
     */
    void setHorizontalAlignment(align_t align);

    /**
     * @brief Set vertical alignment.
     * @details Alignment determines how the object is positioned if the canvas
     *          is larger than the object.
     *
     * @param align [VALIGN_TOP, VALIGN_MIDDLE, VALIGN_BOTTOM]
     */
    void setVerticalAlignment(valign_t align);

    /**
     * @brief Set object width in pixels.
     *
     * @param width Width in pixels.
     */
    void setWidth(uint16_t width);

    /**
     * @brief Set object height in pixels.
     *
     * @param height Height in pixels.
     */
    void setHeight(uint16_t height);

    /**
     * @brief Set inverse status.
     * @details Inverse flag is used for inverting the color of an object.
     *
     * @param inverse Boolean status.
     */
    void setInverse(bool inverse);

    /**
     * @brief Cache control. Set cache permission.
     * @details Static and slow changing objects can be marked as cacheable
     *          so they do not have to be recomputed every time.
     *
     * @param cacheable Boolean flag.
     */
    void setCacheable(bool cacheable);

    /**
     * @brief Cache control. Mark object as invalid.
     * @details Mark object as stale. Any cached versions should be discarded
     *          and a new object should be retrieved.
     */
    void invalidate(void);

    /**
     * @brief Cache control. Get cache permission.
     *
     * @return Boolean cache permission.
     */
    bool isCacheable(void) const;

    /**
     * @brief Cache control. Get validity flag.
     *
     * @return Boolean validity flag.
     */
    bool isValid(void) const;

protected:
    /**
     * @brief UIView constructor.
     */
    UIView(void);

    /**
     * @brief UIView constructor.
     *
     * @param hAlign  Horizontal alignment.
     * @param vAlign  Vertical alignment.
     * @param width   Width in pixels.
     * @param height  Height in pixels.
     * @param inverse Invert colors.
     */
    UIView(align_t hAlign,
           valign_t vAlign,
           uint16_t width,
           uint16_t height,
           bool inverse);

protected:
    /**
     * @brief Get current time in milliseconds.
     * @details Function can be used for controlling animation and progress.
     *
     * @return Current time in milliseconds.
     */
    uint32_t getTimeInMilliseconds(void) const;

    align_t  align;
    valign_t valign;
    uint16_t width;
    uint16_t height;
    bool     inverse;

    /* Is UIView cacheable and is it still valid. */
    bool cacheable;
    bool valid;

    // Callback function for requesting screen update
    FunctionPointer wakeupCallback;
};


#endif // __UIVIEW_H__
