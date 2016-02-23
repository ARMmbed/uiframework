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

#include "UIFramework/UIView.h"

#include <sys/time.h>


UIView::UIView()
    :   align(ALIGN_CENTER),
        valign(VALIGN_MIDDLE),
        width(0),
        height(0),
        inverse(false),
        cacheable(true),
        valid(true)
{
}

UIView::UIView(align_t _hAlign, valign_t _vAlign, uint16_t _width, uint16_t _height, bool _inverse)
    :   align(_hAlign),
        valign(_vAlign),
        width(_width),
        height(_height),
        inverse(_inverse),
        cacheable(true),
        valid(true)
{
}

uint32_t UIView::getTimeInMilliseconds() const
{
    struct timeval tvs;

    gettimeofday(&tvs, NULL);

    return (tvs.tv_sec * 1000) + (tvs.tv_usec / 1000);
}

void UIView::setWakeupCallback(FunctionPointer& wakeup)
{
    wakeupCallback = wakeup;
}


UIView::align_t UIView::getHorizontalAlignment() const
{
    return align;
}

UIView::valign_t UIView::getVerticalAlignment() const
{
    return valign;
}

uint16_t UIView::getWidth() const
{
    return width;
}

uint16_t UIView::getHeight() const
{
    return height;
}

bool UIView::getInverse() const
{
    return inverse;
}

void UIView::setHorizontalAlignment(align_t _align)
{
    align = _align;
}

void UIView::setVerticalAlignment(valign_t _valign)
{
    valign = _valign;
}

void UIView::setWidth(uint16_t _width)
{
    width = _width;
}

void UIView::setHeight(uint16_t _height)
{
    height = _height;
}

void UIView::setInverse(bool _inverse)
{
    inverse = _inverse;
}

/* Cache control
*/
void UIView::setCacheable(bool _cacheable)
{
    cacheable = _cacheable;
}

void UIView::invalidate()
{
    valid = false;
}

bool UIView::isCacheable() const
{
    return cacheable;
}

bool UIView::isValid() const
{
    return valid;
}

SharedPointer<UIView::Action> UIView::getAction()
{
    UIView::Action retval;

    return SharedPointer<UIView::Action>(new UIView::Action());
}




UIView::Action::Action(type_t _type)
    :   type(_type)
{}

UIView::Action::Action(SharedPointer<UIView::Array>& _array)
    :   type(UIView::Action::Array),
        array(_array)
{}

UIView::Action::Action(SharedPointer<UIView>& _view)
    :   type(UIView::Action::View),
        view(_view)
{}

SharedPointer<UIView::Array> UIView::Action::getArray() const
{
    return array;
}

SharedPointer<UIView> UIView::Action::getView() const
{
    return view;
}

UIView::Action::type_t UIView::Action::getType() const
{
    return type;
}


