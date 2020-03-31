
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
#include "cvrt.h"
#include "gtkmain.h"
#include "gtkcv.h"


//-------------------------------------------------------------------------
// Subwidget group for layer list

llist_t::llist_t()
{
    GtkWidget *tform = gtk_table_new(1, 1, false);
    gtk_widget_show(tform);

    GtkWidget *row = gtk_hbox_new(false, 2);
    gtk_widget_show(row);
    GtkWidget *label = gtk_label_new("Layer List (physical)");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(row), label, true, true, 0);
    ll_luse = gtk_check_button_new_with_label("Layers only");
    gtk_widget_set_name(ll_luse, "luse");
    gtk_widget_show(ll_luse);
    g_signal_connect(G_OBJECT(ll_luse), "clicked",
        G_CALLBACK(ll_action), this);
    gtk_box_pack_start(GTK_BOX(row), ll_luse, true, true, 0);

    ll_lskip = gtk_check_button_new_with_label("Skip layers");
    gtk_widget_set_name(ll_lskip, "lskip");
    gtk_widget_show(ll_lskip);
    g_signal_connect(G_OBJECT(ll_lskip), "clicked",
        G_CALLBACK(ll_action), this);
    gtk_box_pack_start(GTK_BOX(row), ll_lskip, true, true, 0);

    int rowcnt = 0;
    gtk_table_attach(GTK_TABLE(tform), row, 0, 2, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);
    rowcnt++;

    ll_laylist = gtk_entry_new();
    gtk_widget_show(ll_laylist);
    gtk_table_attach(GTK_TABLE(tform), ll_laylist, 0, 2, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_FILL),
        (GtkAttachOptions)0, 2, 2);
    rowcnt++;

    ll_aluse = gtk_check_button_new_with_label("Use Layer Aliases");
    gtk_widget_show(ll_aluse);
    gtk_widget_set_name(ll_aluse, "aluse");
    g_signal_connect(G_OBJECT(ll_aluse), "clicked",
        G_CALLBACK(ll_action), this);
    gtk_table_attach(GTK_TABLE(tform), ll_aluse, 0, 1, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    ll_aledit = gtk_toggle_button_new_with_label("Edit Layer Aliases");
    gtk_widget_show(ll_aledit);
    gtk_widget_set_name(ll_aledit, "aledit");
    g_signal_connect(G_OBJECT(ll_aledit), "clicked",
        G_CALLBACK(ll_action), this);
    gtk_table_attach(GTK_TABLE(tform), ll_aledit, 1, 2, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);

    update();
    // must be done after entry text set
    g_signal_connect(G_OBJECT(ll_laylist), "changed",
        G_CALLBACK(ll_text_changed), this);

    ll_frame = gtk_frame_new(0);
    gtk_widget_show(ll_frame);
    gtk_container_add(GTK_CONTAINER(ll_frame), tform);
}


llist_t::~llist_t()
{
    if (ll_aledit && GRX->GetStatus(ll_aledit))
        GRX->CallCallback(ll_aledit);
}


void
llist_t::update()
{
    const char *use = CDvdb()->getVariable(VA_UseLayerList);
    if (use) {
        if (*use == 'n' || *use == 'N') {
            GRX->SetStatus(ll_lskip, true);
            GRX->SetStatus(ll_luse, false);
        }
        else {
            GRX->SetStatus(ll_lskip, false);
            GRX->SetStatus(ll_luse, true);
        }
    }
    else {
        GRX->SetStatus(ll_lskip, false);
        GRX->SetStatus(ll_luse, false);
    }
    const char *list = CDvdb()->getVariable(VA_LayerList);
    if (!list)
        list = "";
    const char *l = gtk_entry_get_text(GTK_ENTRY(ll_laylist));
    if (!l)
        l = "";
    if (strcmp(list, l))
        gtk_entry_set_text(GTK_ENTRY(ll_laylist), list);
    if (GRX->GetStatus(ll_luse) || GRX->GetStatus(ll_lskip))
        gtk_widget_set_sensitive(ll_laylist, true);
    else
        gtk_widget_set_sensitive(ll_laylist, false);

    GRX->SetStatus(ll_aluse, CDvdb()->getVariable(VA_UseLayerAlias));
}


void
llist_t::text_changed(GtkWidget *caller)
{
    if (caller == ll_laylist) {
        const char *s = gtk_entry_get_text(GTK_ENTRY(caller));
        const char *ss = CDvdb()->getVariable(VA_LayerList);
        if (s && *s) {
            if (!ss || strcmp(ss, s))
                CDvdb()->setVariable(VA_LayerList, s);
        }
        else
            CDvdb()->clearVariable(VA_LayerList);
    }
}


void
llist_t::action(GtkWidget *caller)
{
    const char *name = gtk_widget_get_name(caller);
    if (!strcmp(name, "luse")) {
        if (GRX->GetStatus(caller)) {
            CDvdb()->setVariable(VA_UseLayerList, 0);
            // Give the list entry the focus.
            GtkWidget *parent = gtk_widget_get_parent(ll_frame);
            if (parent) {
                while (gtk_widget_get_parent(parent))
                    parent = gtk_widget_get_parent(parent);
                gtk_window_set_focus(GTK_WINDOW(parent), ll_laylist);
            }
        }
        else
            CDvdb()->clearVariable(VA_UseLayerList);
    }
    if (!strcmp(name, "lskip")) {
        if (GRX->GetStatus(caller)) {
            CDvdb()->setVariable(VA_UseLayerList, "n");
            GtkWidget *parent = gtk_widget_get_parent(ll_frame);
            if (parent) {
                while (gtk_widget_get_parent(parent))
                    parent = gtk_widget_get_parent(parent);
                gtk_window_set_focus(GTK_WINDOW(parent), ll_laylist);
            }
        }
        else
            CDvdb()->clearVariable(VA_UseLayerList);
    }
    if (!strcmp(name, "aluse")) {
        if (GRX->GetStatus(caller))
            CDvdb()->setVariable(VA_UseLayerAlias, 0);
        else
            CDvdb()->clearVariable(VA_UseLayerAlias);
    }
    if (!strcmp(name, "aledit")) {
        if (GRX->GetStatus(caller))
            XM()->PopUpLayerAliases(ll_aledit, MODE_ON);
        else
            XM()->PopUpLayerAliases(0, MODE_OFF);
    }
}


// Static function.
void
llist_t::ll_text_changed(GtkWidget *caller, void *arg)
{
    ((llist_t*)arg)->text_changed(caller);
}


// Static function.
void
llist_t::ll_action(GtkWidget *caller, void *arg)
{
    ((llist_t*)arg)->action(caller);
}

