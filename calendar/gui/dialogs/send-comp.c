/* Evolution calendar - Send calendar component dialog
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Author: JP Rosevear <jpr@ximian.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gtk/gtkmessagedialog.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-uidefs.h>
#include <e-util/e-icon-factory.h>
#include "send-comp.h"



/**
 * send_component_dialog:
 * 
 * Pops up a dialog box asking the user whether he wants to send a
 * iTip/iMip message
 * 
 * Return value: TRUE if the user clicked Yes, FALSE otherwise.
 **/
gboolean
send_component_dialog (GtkWindow *parent, ECal *client, ECalComponent *comp, gboolean new)
{
	GtkWidget *dialog;
	ECalComponentVType vtype;
	char *str;
	gint response;

	if (e_cal_get_save_schedules (client))
		return FALSE;
	
	vtype = e_cal_component_get_vtype (comp);

	switch (vtype) {
	case E_CAL_COMPONENT_EVENT:
		if (new)
			str = g_strdup_printf (_("The meeting information has "
						 "been created. Send it?"));
		else
			str = g_strdup_printf (_("The meeting information has "
						 "changed. Send an updated "
						 "version?"));
		break;

	case E_CAL_COMPONENT_TODO:
		if (new)
			str = g_strdup_printf (_("The task assignment "
						 "information has been "
						 "created. Send it?"));
		else
			str = g_strdup_printf (_("The task information has "
						 "changed. Send an updated "
						 "version?"));
		break;

	default:
		g_message ("send_component_dialog(): "
			   "Cannot handle object of type %d", vtype);
		return FALSE;
	}
	
	dialog = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL,
					 GTK_MESSAGE_QUESTION,
					 GTK_BUTTONS_YES_NO, str);
	if (vtype == E_CAL_COMPONENT_EVENT)
		gtk_window_set_icon (GTK_WINDOW (dialog), e_icon_factory_get_icon ("stock_calendar", 32));
	else if (vtype == E_CAL_COMPONENT_TODO)
		gtk_window_set_icon (GTK_WINDOW (dialog), e_icon_factory_get_icon ("stock_todo", 32));

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	if (response == GTK_RESPONSE_YES)
		return TRUE;
	else
		return FALSE;
}
