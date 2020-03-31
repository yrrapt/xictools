
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
 * MOZY html help viewer files                                            *
 *                                                                        *
 *========================================================================*
 $Id:$
 *========================================================================*/

/*------------------------------------------------------------------------*
 * This file is part of the gtkhtm widget library.  The gtkhtm library
 * was derived from the gtk-xmhtml library by:
 *
 *   Stephen R. Whiteley  <stevew@wrcad.com>
 *   Whiteley Research Inc.
 *------------------------------------------------------------------------*
 *  The gtk-xmhtml widget was derived from the XmHTML library by
 *  Miguel de Icaza  <miguel@nuclecu.unam.mx> and others from the GNOME
 *  project.
 *  11/97 - 2/98
 *------------------------------------------------------------------------*
 * Author:  newt
 * (C)Copyright 1995-1996 Ripley Software Development
 * All Rights Reserved
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *------------------------------------------------------------------------*/

#include "htm_widget.h"
#include "htm_image.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>


namespace {
    // Grab one pixel value normalized to maxval, advance the pointer,
    // and return the value normalized to 255.
    //
    unsigned char getval(const unsigned char **s, int maxval)
    {
        if (maxval == 255)
            return (*(*s)++);
        if (maxval < 255) {
            int n = *(*s)++;
            return ((n*255)/maxval);
        }
        int n = *(*s)++;
        n = (n < 8) | *(*s)++;
        return ((n*255)/maxval);
    }
}


htmRawImageData*
htmImageManager::readPPM(ImageBuffer *ib)
{
    const char *s = (char*)ib->buffer;
    if (*s++ != 'P')
        return (0);
    if (!isdigit(*s))
        return (0);
    int index = *s++ - '0';
    switch (index) {
    case 6:
    case 5:
        break;
    default:
        return (0);
    }
    int width = 0;
    int height = 0;
    int maxval = 0;
    for (;;) {
        while (isspace(*s))
            s++;
        if (*s == '#') {
            while (*s && *s != '\n')
                s++;
            continue;
        }
        if (!*s)
            return (0);
        int val = atoi(s);
        if (val < 1)
            return (0);
        while (*s && !isspace(*s))
            s++;
        if (!*s)
            return (0);
        if (!width) {
            width = val;
            continue;
        }
        if (!height) {
            height = val;
            continue;
        }
        if (!maxval) {
            maxval = val;
            s++;
            break;
        }
    }

    if (width < 1 || height < 1 || maxval < 1)
        return (0);

    // allocate raw image data
    htmRawImageData *img_data = new htmRawImageData();
    img_data->width = width;
    img_data->height = height;
    img_data->color_class = IMAGE_COLORSPACE_RGB;
    int linesz = width*3;
    unsigned char *data = new unsigned char[height * linesz];

    if (index == 6) {
        // portable pixel map
        int ix = 0;
        const unsigned char *u = (unsigned char*)s;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                data[ix++] = getval(&u, maxval);
                data[ix++] = getval(&u, maxval);
                data[ix++] = getval(&u, maxval);
            }
        }
    }
    else if (index == 5) {
        // portable gray map
        int ix = 0;
        const unsigned char *u = (unsigned char*)s;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                unsigned char v = getval(&u, maxval);
                data[ix++] = v;
                data[ix++] = v;
                data[ix++] = v;
            }
        }
    }

    // Convert to paletted image.  First allocate a buffer which
    // will receive the final image data.

    img_gtk_selection_data_get_data(data) = new unsigned char[width*height];
    convert24to8(data, img_data,
        im_html->htm_max_image_colors, im_html->htm_rgb_conv_mode);
    delete [] data;
    return (img_data);
}

