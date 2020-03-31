
/*========================================================================*
 *                                                                        *
 *  Distributed by Whiteley Research Inc., Sunnyvale, California, USA     *
 *                       http://wrcad.com                                 *
 *  Copyright (C) 2017 Whiteley Research Inc., all rights reserved.       *
 *  Author: Stephen R. Whiteley, except as indicated.                     *
 *                                                                        *
 *  As fully as possible recognizing licensing terms and conditions       *
 *  imposed by earlier work from which this work was derived, if any,     *
 *  this work is released under the Apache License, Version 2.0 (the      *
 *  "License").  You may not use this file except in compliance with      *
 *  the License, and compliance with inherited licenses which are         *
 *  specified in a sub-header below this one if applicable.  A copy       *
 *  of the License is provided with this distribution, or you may         *
 *  obtain a copy of the License at                                       *
 *                                                                        *
 *        http://www.apache.org/licenses/LICENSE-2.0                      *
 *                                                                        *
 *  See the License for the specific language governing permissions       *
 *  and limitations under the License.                                    *
 *                                                                        *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,      *
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES      *
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-        *
 *   INFRINGEMENT.  IN NO EVENT SHALL WHITELEY RESEARCH INCORPORATED      *
 *   OR STEPHEN R. WHITELEY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER     *
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,      *
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE       *
 *   USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                        *
 *========================================================================*
 *               XicTools Integrated Circuit Design System                *
 *                                                                        *
 * Xic Integrated Circuit Layout and Schematic Editor                     *
 *                                                                        *
 *========================================================================*
 $Id:$
 *========================================================================*/

#include "main.h"
#include "dsp_inlines.h"
#include "fio.h"
#include "keymap.h"
#include "promptline.h"
#include "tech.h"
#include "errorlog.h"
#include "gtkmain.h"
#include "gtkinlines.h"

#include <gdk/gdkkeysyms.h>
#ifdef WITH_X11
#include "gtkinterf/gtkx11.h"
#else
#ifdef WIN32
#include <windows.h>
#endif
#endif


//
// Functions for keyboard mapping and macros
//

namespace {
    //  Mapping for key-down.
    //
    keymap kmap_dn[] = {
        { GDK_KEY_Return,       RETURN_KEY,     0 },
        { GDK_KEY_Escape,       ESCAPE_KEY,     0 },
        { GDK_KEY_Tab,          TAB_KEY,        0 },
        { GDK_KEY_ISO_Left_Tab, TAB_KEY,        0 },
        { GDK_KEY_Break,        BREAK_KEY,      0 },
        { GDK_KEY_Delete,       DELETE_KEY,     0 },
        { GDK_KEY_BackSpace,    BSP_KEY,        0 },
        { GDK_KEY_Left,         LEFT_KEY,       0 },
        { GDK_KEY_Up,           UP_KEY,         0 },
        { GDK_KEY_Right,        RIGHT_KEY,      0 },
        { GDK_KEY_Down,         DOWN_KEY,       0 },
        { GDK_KEY_Shift_L,      SHIFTDN_KEY,    0 },
        { GDK_KEY_Shift_R,      SHIFTDN_KEY,    0 },
        { GDK_KEY_Control_L,    CTRLDN_KEY,     0 },
        { GDK_KEY_Control_R,    CTRLDN_KEY,     0 },
        { GDK_KEY_Home,         HOME_KEY,       0 },
        { GDK_KEY_KP_Add,       NUPLUS_KEY,     0 },
        { GDK_KEY_KP_Subtract,  NUMINUS_KEY,    0 },
#ifdef __APPLE__
        // Mappings for MacBook Pro keyboard: fn-Enter and fn-Right.
        { GDK_KEY_KP_Enter,     NUPLUS_KEY,     0 },
        { GDK_KEY_End,          NUMINUS_KEY,    0 },
#endif
        { GDK_KEY_Next,         PAGEDN_KEY,     0 },
        { GDK_KEY_Prior,        PAGEUP_KEY,     0 },
        { GDK_KEY_F1,           FUNC_KEY,       0 },
        { GDK_KEY_F2,           FUNC_KEY,       1 },
        { GDK_KEY_F3,           FUNC_KEY,       2 },
        { GDK_KEY_F4,           FUNC_KEY,       3 },
        { GDK_KEY_F5,           FUNC_KEY,       4 },
        { GDK_KEY_F6,           FUNC_KEY,       5 },
        { GDK_KEY_F7,           FUNC_KEY,       6 },
        { GDK_KEY_F8,           FUNC_KEY,       7 },
        { GDK_KEY_F9,           FUNC_KEY,       8 },
        { GDK_KEY_F10,          FUNC_KEY,       9 },
        { GDK_KEY_F11,          FUNC_KEY,       10 },
        { GDK_KEY_F12,          FUNC_KEY,       11 },
        { 0,                    NO_KEY,         0 }
    };

    //  Mapping for key-up.
    //
    keymap kmap_up[] = {
        { GDK_KEY_Shift_L,      SHIFTUP_KEY,    0 },
        { GDK_KEY_Shift_R,      SHIFTUP_KEY,    0 },
        { GDK_KEY_Control_L,    CTRLUP_KEY,     0 },
        { GDK_KEY_Control_R,    CTRLUP_KEY,     0 },
        { 0,                    NO_KEY,         0 }
    };

    // Actions done before passing the keypress to the current command.
    //
    keyaction actions_pre[] = {
        { ESCAPE_KEY,  0,                               Escape_action },
        { PAGEDN_KEY,  0,                               DRCf_action },
        { PAGEUP_KEY,  0,                               DRCb_action },
        { TAB_KEY,     GR_SHIFT_MASK,                   CodeRedo_action },
        { TAB_KEY,     0,                               CodeUndo_action },
        { 'c',         GR_CONTROL_MASK,                 Interrupt_action },
        { 'd',         GR_CONTROL_MASK,                 Delete_action },
        { 'g',         GR_CONTROL_MASK,                 Grid_action },
        { 'r',         GR_CONTROL_MASK,                 Redisplay_action },
        { 'x',         GR_CONTROL_MASK,                 Expand_action },
        { 'z',         GR_CONTROL_MASK,                 Iconify_action },
        { 0,           0,                               No_action }
    };

    // Actions performed if keypress was not handled by current command.
    //
    keyaction actions_post[] = {
        { BSP_KEY,     0,                               Bsp_action },
        { DELETE_KEY,  0,                               No_action },
        { UNDO_KEY,    0,                               Undo_action },
        { REDO_KEY,    0,                               Redo_action },
        { 'A',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'B',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'C',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'D',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'E',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'F',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'G',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'H',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'I',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'J',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'K',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'L',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'M',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'N',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'O',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'P',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'Q',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'R',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'S',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'T',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'U',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'V',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'W',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'X',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'Y',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'Z',         GR_SHIFT_MASK | GR_CONTROL_MASK, NameView_action },
        { 'b',         GR_CONTROL_MASK,                 DRCb_action },
        { 'e',         GR_CONTROL_MASK,                 Coord_action },
        { 'f',         GR_CONTROL_MASK,                 DRCf_action },
        { 'n',         GR_CONTROL_MASK,                 SaveView_action },
        { 'p',         GR_CONTROL_MASK,                 DRCp_action },
        { 'u',         GR_CONTROL_MASK,                 ClearKeys_action },
        { 'v',         GR_CONTROL_MASK,                 Version_action },
        { '!',         0,                               Command_action },
        { '?',         0,                               Help_action },
        { LEFT_KEY,    GR_SHIFT_MASK | GR_CONTROL_MASK, SetPrevView_action },
        { LEFT_KEY,    GR_CONTROL_MASK,                 DecRot_action },
        { LEFT_KEY,    GR_SHIFT_MASK,                   PanLeftFine_action },
        { LEFT_KEY,    0,                               PanLeft_action },
        { DOWN_KEY,    GR_SHIFT_MASK | GR_CONTROL_MASK, DecExpand_action },
        { DOWN_KEY,    GR_CONTROL_MASK,                 FlipX_action },
        { DOWN_KEY,    GR_SHIFT_MASK,                   PanDownFine_action },
        { DOWN_KEY,    0,                               PanDown_action },
        { RIGHT_KEY,   GR_SHIFT_MASK | GR_CONTROL_MASK, SetNextView_action },
        { RIGHT_KEY,   GR_CONTROL_MASK,                 IncRot_action },
        { RIGHT_KEY,   GR_SHIFT_MASK,                   PanRightFine_action },
        { RIGHT_KEY,   0,                               PanRight_action },
        { UP_KEY,      GR_SHIFT_MASK | GR_CONTROL_MASK, IncExpand_action },
        { UP_KEY,      GR_CONTROL_MASK,                 FlipY_action },
        { UP_KEY,      GR_SHIFT_MASK,                   PanUpFine_action },
        { UP_KEY,      0,                               PanUp_action },
        { HOME_KEY,    0,                               FullView_action },
        { NUPLUS_KEY,  GR_SHIFT_MASK,                   ZoomInFine_action },
        { NUMINUS_KEY, GR_SHIFT_MASK,                   ZoomOutFine_action },
        { NUPLUS_KEY,  0,                               ZoomIn_action },
        { NUMINUS_KEY, 0,                               ZoomOut_action },
        { 0,           0,                               No_action }
    };
}


void
cKsMap::SetMap(int code, unsigned keyval)
{
    for (keymap *k = KeymapDown(); k->keyval; k++) {
        if (k->code == code)  {
            k->keyval = keyval;
            break;
        }
    }
}


char *
cKsMap::KeyvalToString(unsigned int keyval)
{
    return (gdk_keyval_name(keyval));
}


unsigned int
cKsMap::StringToKeyval(const char *string)
{
    return (gdk_keyval_from_name(string));
}


/*========================================================================
  Keyboard Mapping
 ========================================================================*/

void
cKsMap::init()
{
    kmKeyMapDn = kmap_dn;
    kmKeyMapUp = kmap_up;
    kmActionsPre = actions_pre;
    kmActionsPost = actions_post;
    kmSuppressChar = '`';

    kmKeyTab[0] = sKsMapElt("Home", HOME_KEY, GDK_KEY_Home);
#ifdef GDK_Page_Up
    kmKeyTab[1] = sKsMapElt("Page Up", PAGEUP_KEY, GDK_Page_Up);
#else
    kmKeyTab[1] = sKsMapElt("Page Up", PAGEUP_KEY, 0xffff);
#endif
#ifdef GDK_Page_Down
    kmKeyTab[2] = sKsMapElt("Page Down", PAGEDN_KEY, GDK_Page_Down);
#else
    kmKeyTab[2] = sKsMapElt("Page Down", PAGEDN_KEY, 0xffff);
#endif
#ifdef GDK_KEY_KP_Subtract
    kmKeyTab[3] = sKsMapElt("Numeric Minus", NUMINUS_KEY, GDK_KEY_KP_Subtract);
#else
    kmKeyTab[3] = sKsMapElt("Numeric Minus", NUMINUS_KEY, 0xffff);
#endif
#ifdef GDK_KEY_KP_Add
    kmKeyTab[4] = sKsMapElt("Numeric Plus", NUPLUS_KEY, GDK_KEY_KP_Add);
#else
    kmKeyTab[4] = sKsMapElt("Numeric Plus", NUPLUS_KEY, 0xffff);
#endif
    kmKeyTab[5] = sKsMapElt(0, 0, 0);
}


int
cKsMap::filter_key(unsigned keysym)
{
    switch (keysym) {
    case GDK_KEY_Escape:
        return (-1);
    case GDK_KEY_Return:
        return (1);
    case GDK_KEY_Shift_L:
    case GDK_KEY_Shift_R:
    case GDK_KEY_Control_L:
    case GDK_KEY_Control_R:
    case GDK_KEY_Caps_Lock:
    case GDK_KEY_Shift_Lock:
    case GDK_KEY_Meta_L:
    case GDK_KEY_Meta_R:
    case GDK_KEY_Alt_L:
    case GDK_KEY_Alt_R:
    case GDK_KEY_Super_L:
    case GDK_KEY_Super_R:
    case GDK_KEY_Hyper_L:
    case GDK_KEY_Hyper_R:
    case GDK_KEY_Tab:
    case GDK_KEY_Break:
    case GDK_KEY_Delete:
    case GDK_KEY_Left:
    case GDK_KEY_Up:
    case GDK_KEY_Right:
    case GDK_KEY_Down:
    case GDK_KEY_F1:
    case GDK_KEY_F2:
    case GDK_KEY_F3:
    case GDK_KEY_F4:
    case GDK_KEY_F5:
    case GDK_KEY_F6:
    case GDK_KEY_F7:
    case GDK_KEY_F8:
    case GDK_KEY_F9:
    case GDK_KEY_F10:
    case GDK_KEY_F11:
    case GDK_KEY_F12:
        return (0);
    default:
        if (keysym >= 0xf000)
            return (2);
    }
    return (0);
}

