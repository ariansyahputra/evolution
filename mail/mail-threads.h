/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Author :
 *  Peter Williams (peterw@helixcode.com)
 *
 *  Copyright 2000, Helix Code, Inc. (http://www.helixcode.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef _MAIL_THREADS_H_
#define _MAIL_THREADS_H_

/* Schedule to operation to happen eventually */

gboolean mail_operation_try( const gchar *description, 
			     void (*callback)( gpointer ), 
			     gpointer user_data );

/* User interface hooks for the other thread */

void mail_op_set_percentage( gfloat percentage );
void mail_op_hide_progressbar( void );
void mail_op_show_progressbar( void );
void mail_op_set_message( gchar *fmt, ... ) G_GNUC_PRINTF( 1, 2 );
void mail_op_error( gchar *fmt, ... ) G_GNUC_PRINTF( 1, 2 );
gboolean mail_op_get_password( gchar *prompt, gboolean secret, gchar **dest );

/* Wait for the async operations to finish */
void mail_operation_wait_for_finish( void );

gboolean mail_operations_are_executing( void );

#endif /* defined _MAIL_THREADS_H_ */
