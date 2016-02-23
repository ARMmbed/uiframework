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

#ifndef __UITEXTMONITORVIEW_H__
#define __UITEXTMONITORVIEW_H__


#include "UIFramework/UIView.h"
#include "UIFramework/UITextView.h"

#include <stdio.h>

template <typename T>
class UITextMonitorView : public UIView
{
public:
    /*
        Create Text Monitor using a pointer to a variable.
    */
    UITextMonitorView(const T* _variable,
                      const char* _format,
                      const struct FontData* _font,
                      uint32_t _interval = 1000)
        :   UIView(),
            format(_format),
            font(_font),
            variable(_variable),
            intervalInMilliseconds(_interval),
            callCounter(0),
            variableString()
    {
        MBED_ASSERT(_format);
        MBED_ASSERT(_variable);
        MBED_ASSERT(_font);

        getCurrentValue.attach(this, &UITextMonitorView::getVariableValue);

        previousValue = *_variable;

        updateImage(previousValue);
    }

    /*
        Create Text Monitor using an object and member callback function.
    */
    template <class C>
    UITextMonitorView(C* object,
                      T (C::*member)(void),
                      const char* _format,
                      const struct FontData* _font,
                      uint32_t _interval = 1000)
        :   UIView(),
            format(_format),
            font(_font),
            variable(NULL),
            intervalInMilliseconds(_interval),
            callCounter(0),
            variableString()
    {
        MBED_ASSERT(_format);
        MBED_ASSERT(object);
        MBED_ASSERT(member);
        MBED_ASSERT(_font);

        getCurrentValue.attach(object, member);

        previousValue = getCurrentValue.call();

        updateImage(previousValue);
    }

    /*
        Create Text Monitor using callback function.
    */
    UITextMonitorView(T (*callback)(void),
                      const char* _format,
                      const struct FontData* _font,
                      uint32_t _interval = 1000)
        :   UIView(),
            format(_format),
            font(_font),
            variable(NULL),
            intervalInMilliseconds(_interval),
            callCounter(0),
            variableString()
    {
        MBED_ASSERT(_format);
        MBED_ASSERT(callback);
        MBED_ASSERT(_font);

        getCurrentValue.attach(callback);

        previousValue = getCurrentValue.call();

        updateImage(previousValue);
    }

    // from UIView
    virtual ~UITextMonitorView()
    { }

    virtual uint32_t fillFrameBuffer(SharedPointer<FrameBuffer>& canvas, int16_t xOffset, int16_t yOffset)
    {
        /* use current time to evaluate whether image needs to be updated */
        uint32_t now = UIView::getTimeInMilliseconds();

        if ((now - callCounter) > intervalInMilliseconds)
        {
            /* read current value of variable */
            T currentValue = getCurrentValue.call();

            /* update image cell if value has changed */
            if (currentValue != previousValue)
            {
                updateImage(currentValue);

                /* store current value in cache */
                previousValue = currentValue;
            }

            /* reset counter */
            callCounter = now;
        }

        /* copy image to canvas */
        if (variableCell)
        {
            variableCell->setInverse(inverse);
            variableCell->setHorizontalAlignment(align);
            variableCell->setVerticalAlignment(valign);
            variableCell->setWidth(width);
            variableCell->setHeight(height);

            variableCell->fillFrameBuffer(canvas, xOffset, yOffset);
        }

        /* the plus sign allows multiple monitors to sync up with each other. */
        return intervalInMilliseconds + (now - callCounter);
    }

    void setInterval(uint32_t interval)
    {
        intervalInMilliseconds = interval;
    }

private:
    /*
        Internal callback function when monitoring a pointer.
    */
    T getVariableValue()
    {
        return (variable) ? (*variable) : 0;
    }

    /*
        Helper function for creating a static Text image
        using the passed argument as value.
    */
    void updateImage(T value)
    {
        /* create formatted string from argument */
        char buffer[12] = {0};
        snprintf(buffer, 12, format, value);
        variableString = std::string(buffer);

        variableCell = SharedPointer<UITextView>(new UITextView(variableString.c_str(), font));
    }

private:
    const char* format;
    const struct FontData* font;
    const T* variable;
    T previousValue;
    uint32_t intervalInMilliseconds;
    uint32_t callCounter;

    FunctionPointer0<T> getCurrentValue;

    std::string variableString;
    SharedPointer<UITextView> variableCell;
};

#endif // __UITEXTMONITORVIEW_H__
