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


#include "mbed-drivers/mbed.h"

#include "UIFramework/UIFramework.h"
#include "UIFramework/UIImageView.h"
#include "UIFramework/UITextView.h"
#include "UIFramework/UITextMonitorView.h"

#include "UIFramework/UITableView.h"

#include "uif-tools-1bit/fonts/fonts.h"

// screen
static SharedPointer<UIFramework> uiFramework;

static uint32_t counter = 0;

void incrementCounterTask()
{
    counter++;
}

void app_start(int, char *[])
{
    minar::Scheduler::postCallback(incrementCounterTask)
        .period(minar::milliseconds(1000));

    std::string hello("hello");
    SharedPointer<UIView> view(new UITextView(hello, &Font_Menu));
//    SharedPointer<UIView> view(new UITextMonitorView<uint32_t>((uint32_t*) &(counter), "%d", &Font_Menu));

    view->setInverse(false);
    view->setWidth(128);
    view->setHeight(128);

    uiFramework = SharedPointer<UIFramework>(new UIFramework(view));
}
