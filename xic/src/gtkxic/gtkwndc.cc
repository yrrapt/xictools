
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
#include "fio.h"
#include "gtkmain.h"
#include "gtkcv.h"


//-------------------------------------------------------------------------
// Subwidget group for window control

wnd_t::wnd_t(WndSensMode(sens_test)(), WndFuncMode fmode)
{
    wnd_sens_test = sens_test;
    wnd_func_mode = fmode;

    GtkWidget *tform = gtk_table_new(1, 1, false);
    gtk_widget_show(tform);

    GtkWidget *row = gtk_hbox_new(false, 2);
    gtk_widget_show(row);
    wnd_use_win = gtk_check_button_new_with_label("Use Window");
    gtk_widget_set_name(wnd_use_win, "Window");
    gtk_widget_show(wnd_use_win);
    g_signal_connect(G_OBJECT(wnd_use_win), "clicked",
        G_CALLBACK(wnd_action), this);
    gtk_box_pack_start(GTK_BOX(row), wnd_use_win, false, false, 0);

    wnd_clip = gtk_check_button_new_with_label("Clip to Window");
    gtk_widget_set_name(wnd_clip, "Clip");
    gtk_widget_show(wnd_clip);
    g_signal_connect(G_OBJECT(wnd_clip), "clicked",
        G_CALLBACK(wnd_action), this);
    gtk_box_pack_start(GTK_BOX(row), wnd_clip, false, false, 0);

    wnd_flatten = gtk_check_button_new_with_label("Flatten Hierarchy");
    gtk_widget_set_name(wnd_flatten, "Flatten");
    gtk_widget_show(wnd_flatten);
    g_signal_connect(G_OBJECT(wnd_flatten), "clicked",
        G_CALLBACK(wnd_action), this);
    gtk_box_pack_start(GTK_BOX(row), wnd_flatten, false, false, 0);

    int rowcnt = 0;
    gtk_table_attach(GTK_TABLE(tform), row, 0, 4, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    wnd_ecf_label = gtk_label_new("Empty Cell Filter");
    gtk_widget_show(wnd_ecf_label);
    gtk_misc_set_padding(GTK_MISC(wnd_ecf_label), 2, 2);
    if (wnd_func_mode == WndFuncOut || wnd_func_mode == WndFuncIn)
        gtk_widget_hide(wnd_ecf_label);
    gtk_table_attach(GTK_TABLE(tform), wnd_ecf_label, 4, 5, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);
    rowcnt++;

    wnd_sbutton = gtk_button_new_with_label("S");
    gtk_widget_set_name(wnd_sbutton, "SaveWin");
    gtk_widget_show(wnd_sbutton);
    gtk_widget_set_size_request(wnd_sbutton, 20, -1);

    char buf[64];
    wnd_s_menu = gtk_menu_new();
    gtk_widget_set_name(wnd_s_menu, "Smenu");
    for (int i = 0; i < FIO_NUM_BB_STORE; i++) {
        sprintf(buf, "Reg %d", i);
        GtkWidget *mi = gtk_menu_item_new_with_label(buf);
        gtk_widget_set_name(mi, buf);
        gtk_widget_show(mi);
        gtk_menu_shell_append(GTK_MENU_SHELL(wnd_s_menu), mi);
        g_signal_connect(G_OBJECT(mi), "activate",
            G_CALLBACK(wnd_sto_menu_proc), this);
    }
    g_signal_connect(G_OBJECT(wnd_sbutton), "button-press-event",
        G_CALLBACK(wnd_popup_btn_proc), wnd_s_menu);

    wnd_l_label = gtk_label_new("Left");
    gtk_widget_show(wnd_l_label);
    gtk_misc_set_padding(GTK_MISC(wnd_l_label), 2, 2);

    GtkWidget *hbox = gtk_hbox_new(false, 2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(hbox), wnd_sbutton, false, false, 0);
    gtk_box_pack_start(GTK_BOX(hbox), wnd_l_label, true, true, 0);

    gtk_table_attach(GTK_TABLE(tform), hbox, 0, 1, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    int ndgt = CD()->numDigits();

    double initd = 0;
    if (wnd_func_mode == WndFuncCvt)
        initd = MICRONS(FIO()->CvtWindow()->left);
    else if (wnd_func_mode == WndFuncOut)
        initd = MICRONS(FIO()->OutWindow()->left);
    else if (wnd_func_mode == WndFuncIn)
        initd = MICRONS(FIO()->InWindow()->left);

    GtkWidget *sb = sb_left.init(initd, -1e6, 1e6, ndgt);
    // (wnd_val_changed, this, "left");
    gtk_widget_set_size_request(sb, 80, -1);

    gtk_table_attach(GTK_TABLE(tform), sb, 1, 2, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    wnd_b_label = gtk_label_new("Bottom");
    gtk_widget_show(wnd_b_label);
    gtk_misc_set_padding(GTK_MISC(wnd_b_label), 2, 2);
    gtk_table_attach(GTK_TABLE(tform), wnd_b_label, 2, 3, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    initd = 0;
    if (wnd_func_mode == WndFuncCvt)
        initd = MICRONS(FIO()->CvtWindow()->bottom);
    else if (wnd_func_mode == WndFuncOut)
        initd = MICRONS(FIO()->OutWindow()->bottom);
    else if (wnd_func_mode == WndFuncIn)
        initd = MICRONS(FIO()->InWindow()->bottom);

    sb = sb_bottom.init(initd, -1e6, 1e6, ndgt);
    // (wnd_val_changed, this, "bottom");
    gtk_widget_set_size_request(sb, 80, -1);

    gtk_table_attach(GTK_TABLE(tform), sb, 3, 4, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    hbox = gtk_hbox_new(false, 2);
    gtk_widget_show(hbox);
    GtkWidget *label = gtk_label_new("  ");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);

    wnd_ecf_pre = gtk_check_button_new_with_label("pre-filter");
    gtk_widget_set_name(wnd_ecf_pre, "pre");
    gtk_widget_show(wnd_ecf_pre);
    g_signal_connect(G_OBJECT(wnd_ecf_pre), "clicked",
        G_CALLBACK(wnd_action), this);
    if (wnd_func_mode == WndFuncOut || wnd_func_mode == WndFuncIn)
        gtk_widget_hide(hbox);
    gtk_box_pack_start(GTK_BOX(hbox), wnd_ecf_pre, false, false, 0);
    gtk_table_attach(GTK_TABLE(tform), hbox, 4, 5, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);
    rowcnt++;

    wnd_rbutton = gtk_button_new_with_label("R");
    gtk_widget_set_name(wnd_rbutton, "RclWin");
    gtk_widget_show(wnd_rbutton);
    gtk_widget_set_size_request(wnd_rbutton, 20, -1);

    wnd_r_menu = gtk_menu_new();
    gtk_widget_set_name(wnd_r_menu, "Rmenu");
    for (int i = 0; i < FIO_NUM_BB_STORE; i++) {
        sprintf(buf, "Reg %d", i);
        GtkWidget *mi = gtk_menu_item_new_with_label(buf);
        gtk_widget_set_name(mi, buf);
        gtk_widget_show(mi);
        gtk_menu_shell_append(GTK_MENU_SHELL(wnd_r_menu), mi);
        g_signal_connect(G_OBJECT(mi), "activate",
            G_CALLBACK(wnd_rcl_menu_proc), this);
    }
    g_signal_connect(G_OBJECT(wnd_rbutton), "button-press-event",
        G_CALLBACK(wnd_popup_btn_proc), wnd_r_menu);

    wnd_r_label = gtk_label_new("Right");
    gtk_widget_show(wnd_r_label);
    gtk_misc_set_padding(GTK_MISC(wnd_r_label), 2, 2);

    hbox = gtk_hbox_new(false, 2);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(hbox), wnd_rbutton, false, false, 0);
    gtk_box_pack_start(GTK_BOX(hbox), wnd_r_label, true, true, 0);

    gtk_table_attach(GTK_TABLE(tform), hbox, 0, 1, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    initd = 0;
    if (wnd_func_mode == WndFuncCvt)
        initd = MICRONS(FIO()->CvtWindow()->right);
    else if (wnd_func_mode == WndFuncOut)
        initd = MICRONS(FIO()->OutWindow()->right);
    else if (wnd_func_mode == WndFuncIn)
        initd = MICRONS(FIO()->InWindow()->right);

    sb = sb_right.init(initd, -1e6, 1e6, ndgt);
    // (wnd_val_changed, this, "right");
    gtk_widget_set_size_request(sb, 80, -1);

    gtk_table_attach(GTK_TABLE(tform), sb, 1, 2, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    wnd_t_label = gtk_label_new("Top");
    gtk_widget_show(wnd_t_label);
    gtk_misc_set_padding(GTK_MISC(wnd_t_label), 2, 2);
    gtk_table_attach(GTK_TABLE(tform), wnd_t_label, 2, 3, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    initd = 0;
    if (wnd_func_mode == WndFuncCvt)
        initd = MICRONS(FIO()->CvtWindow()->top);
    else if (wnd_func_mode == WndFuncOut)
        initd = MICRONS(FIO()->OutWindow()->top);
    else if (wnd_func_mode == WndFuncIn)
        initd = MICRONS(FIO()->InWindow()->top);

    sb = sb_top.init(initd, -1e6, 1e6, ndgt);
    // (wnd_val_changed, this, "top");
    gtk_widget_set_size_request(sb, 80, -1);

    gtk_table_attach(GTK_TABLE(tform), sb, 3, 4, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    hbox = gtk_hbox_new(false, 2);
    gtk_widget_show(hbox);
    label = gtk_label_new("  ");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);

    wnd_ecf_post = gtk_check_button_new_with_label("post-filter");
    gtk_widget_set_name(wnd_ecf_post, "post");
    gtk_widget_show(wnd_ecf_post);
    g_signal_connect(G_OBJECT(wnd_ecf_post), "clicked",
        G_CALLBACK(wnd_action), this);
    if (wnd_func_mode == WndFuncOut || wnd_func_mode == WndFuncIn)
        gtk_widget_hide(hbox);
    gtk_box_pack_start(GTK_BOX(hbox), wnd_ecf_post, false, false, 0);
    gtk_table_attach(GTK_TABLE(tform), hbox, 4, 5, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    wnd_frame = gtk_frame_new(0);
    gtk_widget_show(wnd_frame);
    gtk_container_add(GTK_CONTAINER(wnd_frame), tform);
    update();
}


wnd_t::~wnd_t()
{
    g_object_ref(wnd_s_menu);
    g_object_ref(G_OBJECT(wnd_s_menu));
    gtk_widget_destroy(wnd_s_menu);
    g_object_unref(wnd_s_menu);

    g_object_ref(wnd_r_menu);
    g_object_ref(G_OBJECT(wnd_r_menu));
    gtk_widget_destroy(wnd_r_menu);
    g_object_unref(wnd_r_menu);
}


void
wnd_t::update()
{
    if (wnd_func_mode == WndFuncCvt) {
        sb_left.set_value(MICRONS(FIO()->CvtWindow()->left));
        sb_bottom.set_value(MICRONS(FIO()->CvtWindow()->bottom));
        sb_right.set_value(MICRONS(FIO()->CvtWindow()->right));
        sb_top.set_value(MICRONS(FIO()->CvtWindow()->top));
        GRX->SetStatus(wnd_use_win, FIO()->CvtUseWindow());
        GRX->SetStatus(wnd_clip, FIO()->CvtClip());
        GRX->SetStatus(wnd_flatten, FIO()->CvtFlatten());
        GRX->SetStatus(wnd_ecf_pre,
            FIO()->CvtECFlevel() == ECFall || FIO()->CvtECFlevel() == ECFpre);
        GRX->SetStatus(wnd_ecf_post,
            FIO()->CvtECFlevel() == ECFall || FIO()->CvtECFlevel() == ECFpost);
    }
    else if (wnd_func_mode == WndFuncOut) {
        sb_left.set_value(MICRONS(FIO()->OutWindow()->left));
        sb_bottom.set_value(MICRONS(FIO()->OutWindow()->bottom));
        sb_right.set_value(MICRONS(FIO()->OutWindow()->right));
        sb_top.set_value(MICRONS(FIO()->OutWindow()->top));
        GRX->SetStatus(wnd_use_win, FIO()->OutUseWindow());
        GRX->SetStatus(wnd_clip, FIO()->OutClip());
        GRX->SetStatus(wnd_flatten, FIO()->OutFlatten());
        GRX->SetStatus(wnd_ecf_pre,
            FIO()->OutECFlevel() == ECFall || FIO()->OutECFlevel() == ECFpre);
        GRX->SetStatus(wnd_ecf_post,
            FIO()->OutECFlevel() == ECFall || FIO()->OutECFlevel() == ECFpost);
    }
    else if (wnd_func_mode == WndFuncIn) {
        sb_left.set_value(MICRONS(FIO()->InWindow()->left));
        sb_bottom.set_value(MICRONS(FIO()->InWindow()->bottom));
        sb_right.set_value(MICRONS(FIO()->InWindow()->right));
        sb_top.set_value(MICRONS(FIO()->InWindow()->top));
        GRX->SetStatus(wnd_use_win, FIO()->InUseWindow());
        GRX->SetStatus(wnd_clip, FIO()->InClip());
        GRX->SetStatus(wnd_flatten, FIO()->InFlatten());
        GRX->SetStatus(wnd_ecf_pre,
            FIO()->InECFlevel() == ECFall || FIO()->InECFlevel() == ECFpre);
        GRX->SetStatus(wnd_ecf_post,
            FIO()->InECFlevel() == ECFall || FIO()->InECFlevel() == ECFpost);
    }
    set_sens();
}


void
wnd_t::val_changed(GtkWidget *caller)
{
    const char *n = gtk_widget_get_name(caller);
    if (*n == 'l') {
        const char *s = sb_left.get_string();
        char *endp;
        double d = strtod(s, &endp);
        if (endp > s) {
            if (wnd_func_mode == WndFuncCvt)
                FIO()->SetCvtWindowLeft(INTERNAL_UNITS(d));
            else if (wnd_func_mode == WndFuncOut)
                FIO()->SetOutWindowLeft(INTERNAL_UNITS(d));
            else if (wnd_func_mode == WndFuncIn)
                FIO()->SetInWindowLeft(INTERNAL_UNITS(d));
        }
    }
    else if (*n == 'b') {
        const char *s = sb_bottom.get_string();
        char *endp;
        double d = strtod(s, &endp);
        if (endp > s) {
            if (wnd_func_mode == WndFuncCvt)
                FIO()->SetCvtWindowBottom(INTERNAL_UNITS(d));
            else if (wnd_func_mode == WndFuncOut)
                FIO()->SetOutWindowBottom(INTERNAL_UNITS(d));
            else if (wnd_func_mode == WndFuncIn)
                FIO()->SetInWindowBottom(INTERNAL_UNITS(d));
        }
    }
    else if (*n == 'r') {
        const char *s = sb_right.get_string();
        char *endp;
        double d = strtod(s, &endp);
        if (endp > s) {
            if (wnd_func_mode == WndFuncCvt)
                FIO()->SetCvtWindowRight(INTERNAL_UNITS(d));
            else if (wnd_func_mode == WndFuncOut)
                FIO()->SetOutWindowRight(INTERNAL_UNITS(d));
            else if (wnd_func_mode == WndFuncIn)
                FIO()->SetInWindowRight(INTERNAL_UNITS(d));
        }
    }
    else if (*n == 't') {
        const char *s = sb_top.get_string();
        char *endp;
        double d = strtod(s, &endp);
        if (endp > s) {
            if (wnd_func_mode == WndFuncCvt)
                FIO()->SetCvtWindowTop(INTERNAL_UNITS(d));
            else if (wnd_func_mode == WndFuncOut)
                FIO()->SetOutWindowTop(INTERNAL_UNITS(d));
            else if (wnd_func_mode == WndFuncIn)
                FIO()->SetInWindowTop(INTERNAL_UNITS(d));
        }
    }
}


void
wnd_t::action(GtkWidget *caller)
{
    const char *name = gtk_widget_get_name(caller);
    if (!strcmp(name, "Window")) {
        if (wnd_func_mode == WndFuncCvt)
            FIO()->SetCvtUseWindow(GRX->GetStatus(caller));
        else if (wnd_func_mode == WndFuncOut)
            FIO()->SetOutUseWindow(GRX->GetStatus(caller));
        else if (wnd_func_mode == WndFuncIn)
            FIO()->SetInUseWindow(GRX->GetStatus(caller));
        set_sens();
    }
    if (!strcmp(name, "Clip")) {
        if (wnd_func_mode == WndFuncCvt)
            FIO()->SetCvtClip(GRX->GetStatus(caller));
        else if (wnd_func_mode == WndFuncOut)
            FIO()->SetOutClip(GRX->GetStatus(caller));
        else if (wnd_func_mode == WndFuncIn)
            FIO()->SetInClip(GRX->GetStatus(caller));
    }
    if (!strcmp(name, "Flatten")) {
        if (wnd_func_mode == WndFuncCvt)
            FIO()->SetCvtFlatten(GRX->GetStatus(caller));
        else if (wnd_func_mode == WndFuncOut)
            FIO()->SetOutFlatten(GRX->GetStatus(caller));
        else if (wnd_func_mode == WndFuncIn)
            FIO()->SetInFlatten(GRX->GetStatus(caller));
        set_sens();
    }
    if (!strcmp(name, "pre")) {
        if (wnd_func_mode == WndFuncCvt) {
            if (GRX->GetStatus(caller)) {
                switch (FIO()->CvtECFlevel()) {
                case ECFnone:
                    FIO()->SetCvtECFlevel(ECFpre);
                    break;
                case ECFall:
                    break;
                case ECFpre:
                    break;
                case ECFpost:
                    FIO()->SetCvtECFlevel(ECFall);
                    break;
                }
            }
            else {
                switch (FIO()->CvtECFlevel()) {
                case ECFnone:
                    break;
                case ECFall:
                    FIO()->SetCvtECFlevel(ECFpost);
                    break;
                case ECFpre:
                    FIO()->SetCvtECFlevel(ECFnone);
                    break;
                case ECFpost:
                    break;
                }
            }
        }
        else if (wnd_func_mode == WndFuncOut) {
            if (GRX->GetStatus(caller)) {
                switch (FIO()->OutECFlevel()) {
                case ECFnone:
                    FIO()->SetOutECFlevel(ECFpre);
                    break;
                case ECFall:
                    break;
                case ECFpre:
                    break;
                case ECFpost:
                    FIO()->SetOutECFlevel(ECFall);
                    break;
                }
            }
            else {
                switch (FIO()->OutECFlevel()) {
                case ECFnone:
                    break;
                case ECFall:
                    FIO()->SetOutECFlevel(ECFpost);
                    break;
                case ECFpre:
                    FIO()->SetOutECFlevel(ECFnone);
                    break;
                case ECFpost:
                    break;
                }
            }
        }
        else if (wnd_func_mode == WndFuncIn) {
            if (GRX->GetStatus(caller)) {
                switch (FIO()->InECFlevel()) {
                case ECFnone:
                    FIO()->SetInECFlevel(ECFpre);
                    break;
                case ECFall:
                    break;
                case ECFpre:
                    break;
                case ECFpost:
                    FIO()->SetInECFlevel(ECFall);
                    break;
                }
            }
            else {
                switch (FIO()->InECFlevel()) {
                case ECFnone:
                    break;
                case ECFall:
                    FIO()->SetInECFlevel(ECFpost);
                    break;
                case ECFpre:
                    FIO()->SetInECFlevel(ECFnone);
                    break;
                case ECFpost:
                    break;
                }
            }
        }
    }
    if (!strcmp(name, "post")) {
        if (wnd_func_mode == WndFuncCvt) {
            if (GRX->GetStatus(caller)) {
                switch (FIO()->CvtECFlevel()) {
                case ECFnone:
                    FIO()->SetCvtECFlevel(ECFpost);
                    break;
                case ECFall:
                    break;
                case ECFpre:
                    FIO()->SetCvtECFlevel(ECFall);
                    break;
                case ECFpost:
                    break;
                }
            }
            else {
                switch (FIO()->CvtECFlevel()) {
                case ECFnone:
                    break;
                case ECFall:
                    FIO()->SetCvtECFlevel(ECFpre);
                    break;
                case ECFpre:
                    break;
                case ECFpost:
                    FIO()->SetCvtECFlevel(ECFnone);
                    break;
                }
            }
        }
        else if (wnd_func_mode == WndFuncOut) {
            if (GRX->GetStatus(caller)) {
                switch (FIO()->OutECFlevel()) {
                case ECFnone:
                    FIO()->SetOutECFlevel(ECFpost);
                    break;
                case ECFall:
                    break;
                case ECFpre:
                    FIO()->SetOutECFlevel(ECFall);
                    break;
                case ECFpost:
                    break;
                }
            }
            else {
                switch (FIO()->OutECFlevel()) {
                case ECFnone:
                    break;
                case ECFall:
                    FIO()->SetOutECFlevel(ECFpre);
                    break;
                case ECFpre:
                    break;
                case ECFpost:
                    FIO()->SetOutECFlevel(ECFnone);
                    break;
                }
            }
        }
        else if (wnd_func_mode == WndFuncIn) {
            if (GRX->GetStatus(caller)) {
                switch (FIO()->InECFlevel()) {
                case ECFnone:
                    FIO()->SetInECFlevel(ECFpost);
                    break;
                case ECFall:
                    break;
                case ECFpre:
                    FIO()->SetInECFlevel(ECFall);
                    break;
                case ECFpost:
                    break;
                }
            }
            else {
                switch (FIO()->InECFlevel()) {
                case ECFnone:
                    break;
                case ECFall:
                    FIO()->SetInECFlevel(ECFpre);
                    break;
                case ECFpre:
                    break;
                case ECFpost:
                    FIO()->SetInECFlevel(ECFnone);
                    break;
                }
            }
        }
    }
}


void
wnd_t::set_sens()
{
    WndSensMode mode = (wnd_sens_test ? (*wnd_sens_test)() : WndSensAllModes);

    // No Empties is visible only when translating.

    if (mode == WndSensAllModes) {
        // Basic mode when translating archive to archive.
        // Flattten, UseWindow are independent.
        // NoEmpties available when not flattening.
        // Clip is enabled by UseWindow.

        bool u = GRX->GetStatus(wnd_use_win);
        if (!u)
            GRX->SetStatus(wnd_clip, false);
        bool f = GRX->GetStatus(wnd_flatten);
        if (f) {
            GRX->SetStatus(wnd_ecf_pre, false);
            GRX->SetStatus(wnd_ecf_post, false);
        }
        gtk_widget_set_sensitive(wnd_use_win, true);
        gtk_widget_set_sensitive(wnd_flatten, true);
        gtk_widget_set_sensitive(wnd_ecf_label, !f);
        gtk_widget_set_sensitive(wnd_ecf_pre, !f);
        gtk_widget_set_sensitive(wnd_ecf_post, !f);
        gtk_widget_set_sensitive(wnd_clip, u);
        sb_left.set_sensitive(u);
        sb_bottom.set_sensitive(u);
        sb_right.set_sensitive(u);
        sb_top.set_sensitive(u);
        gtk_widget_set_sensitive(wnd_l_label, u);
        gtk_widget_set_sensitive(wnd_b_label, u);
        gtk_widget_set_sensitive(wnd_r_label, u);
        gtk_widget_set_sensitive(wnd_t_label, u);
        gtk_widget_set_sensitive(wnd_rbutton, u);
        gtk_widget_set_sensitive(wnd_sbutton, u);
    }
    else if (mode == WndSensFlatten) {
        // Basic mode when writing database to archive or native, and
        // writing CHD to native.
        // UseWindow is available only when flattening.
        // NoEmpties available when not flattening.
        // Clip is enabled by UseWindow.

        bool f = GRX->GetStatus(wnd_flatten);
        if (!f)
            GRX->SetStatus(wnd_use_win, false);
        else {
            GRX->SetStatus(wnd_ecf_pre, false);
            GRX->SetStatus(wnd_ecf_post, false);
        }
        bool u = GRX->GetStatus(wnd_use_win);
        if (!u)
            GRX->SetStatus(wnd_clip, false);
        gtk_widget_set_sensitive(wnd_flatten, true);
        gtk_widget_set_sensitive(wnd_ecf_label, !f);
        gtk_widget_set_sensitive(wnd_ecf_pre, !f);
        gtk_widget_set_sensitive(wnd_ecf_post, !f);
        gtk_widget_set_sensitive(wnd_use_win, f);
        gtk_widget_set_sensitive(wnd_clip, f && u);
        sb_left.set_sensitive(f && u);
        sb_bottom.set_sensitive(f && u);
        sb_right.set_sensitive(f && u);
        sb_top.set_sensitive(f && u);
        gtk_widget_set_sensitive(wnd_l_label, f && u);
        gtk_widget_set_sensitive(wnd_b_label, f && u);
        gtk_widget_set_sensitive(wnd_r_label, f && u);
        gtk_widget_set_sensitive(wnd_t_label, f && u);
        gtk_widget_set_sensitive(wnd_rbutton, f && u);
        gtk_widget_set_sensitive(wnd_sbutton, f && u);
    }
    else if (mode == WndSensNone) {
        // If source is from text, nothing is enabled.

        gtk_widget_set_sensitive(wnd_flatten, false);
        gtk_widget_set_sensitive(wnd_ecf_label, false);
        gtk_widget_set_sensitive(wnd_ecf_pre, false);
        gtk_widget_set_sensitive(wnd_ecf_post, false);
        gtk_widget_set_sensitive(wnd_use_win, false);
        gtk_widget_set_sensitive(wnd_clip, false);
        sb_left.set_sensitive(false);
        sb_bottom.set_sensitive(false);
        sb_right.set_sensitive(false);
        sb_top.set_sensitive(false);
        gtk_widget_set_sensitive(wnd_l_label, false);
        gtk_widget_set_sensitive(wnd_b_label, false);
        gtk_widget_set_sensitive(wnd_r_label, false);
        gtk_widget_set_sensitive(wnd_t_label, false);
        gtk_widget_set_sensitive(wnd_rbutton, false);
        gtk_widget_set_sensitive(wnd_sbutton, false);
    }
}


bool
wnd_t::get_bb(BBox *BB)
{
    if (!BB)
        return (false);
    double l, b, r, t;
    int i = 0;
    i += sscanf(sb_left.get_string(), "%lf", &l);
    i += sscanf(sb_bottom.get_string(), "%lf", &b);
    i += sscanf(sb_right.get_string(), "%lf", &r);
    i += sscanf(sb_top.get_string(), "%lf", &t);
    if (i != 4)
        return (false);
    BB->left = INTERNAL_UNITS(l);
    BB->bottom = INTERNAL_UNITS(b);
    BB->right = INTERNAL_UNITS(r);
    BB->top = INTERNAL_UNITS(t);
    if (BB->right < BB->left) {
        int tmp = BB->left;
        BB->left = BB->right;
        BB->right = tmp;
    }
    if (BB->top < BB->bottom) {
        int tmp = BB->bottom;
        BB->bottom = BB->top;
        BB->top = tmp;
    }
    if (BB->left == BB->right || BB->bottom == BB->top)
        return (false);;
    return (true);
}


void
wnd_t::set_bb(const BBox *BB)
{
    if (!BB)
        return;
    sb_left.set_value(MICRONS(BB->left));
    sb_bottom.set_value(MICRONS(BB->bottom));
    sb_right.set_value(MICRONS(BB->right));
    sb_top.set_value(MICRONS(BB->top));
}


// Static function.
void
wnd_t::wnd_val_changed(GtkWidget *caller, void *arg)
{
    ((wnd_t*)arg)->val_changed(caller);
}


// Static function.
void
wnd_t::wnd_action(GtkWidget *caller, void *arg)
{
    ((wnd_t*)arg)->action(caller);
}


namespace {
    // Positioning function for pop-up menus.
    //
    void
    pos_func(GtkMenu*, int *x, int *y, gboolean *pushin, void *data)
    {
        *pushin = true;
        GtkWidget *btn = GTK_WIDGET(data);
        GRX->Location(btn, x, y);
    }
}


// Static function.
// Button-press handler to produce pop-up menu.
//
int
wnd_t::wnd_popup_btn_proc(GtkWidget *widget, GdkEvent *event, void *arg)
{
    gtk_menu_popup(GTK_MENU(arg), 0, 0, pos_func, widget, event->button.button,
        event->button.time);
    return (true);
}


// Static function.
void
wnd_t::wnd_sto_menu_proc(GtkWidget *widget, void *arg)
{
    wnd_t *wnd = (wnd_t*)arg;
    const char *name = gtk_widget_get_name(widget);
    if (name && wnd) {
        while (*name) {
            if (isdigit(*name))
                break;
            name++;
        }
        if (isdigit(*name))
            wnd->get_bb(FIO()->savedBB(*name - '0'));
    }
}


// Static function.
void
wnd_t::wnd_rcl_menu_proc(GtkWidget *widget, void *arg)
{
    wnd_t *wnd = (wnd_t*)arg;
    const char *name = gtk_widget_get_name(widget);
    if (name && wnd) {
        while (*name) {
            if (isdigit(*name))
                break;
            name++;
        }
        if (isdigit(*name))
            wnd->set_bb(FIO()->savedBB(*name - '0'));
    }
}

