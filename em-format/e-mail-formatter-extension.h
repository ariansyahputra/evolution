/*
 * e-mail-formatter-extension.h
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the program; if not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef E_MAIL_FORMATTER_EXTENSION_H
#define E_MAIL_FORMATTER_EXTENSION_H

#include <em-format/e-mail-part.h>
#include <em-format/e-mail-formatter.h>
#include <camel/camel.h>
#include <gtk/gtk.h>

/* Standard GObject macros */
#define E_TYPE_MAIL_FORMATTER_EXTENSION \
	(e_mail_formatter_extension_get_type ())
#define E_MAIL_FORMATTER_EXTENSION(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), E_TYPE_MAIL_FORMATTER_EXTENSION, EMailFormatterExtension))
#define E_MAIL_FORMATTER_EXTENSION_INTERFACE(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), E_TYPE_MAIL_FORMATTER_EXTENSION, EMailFormatterExtensionInterface))
#define E_IS_MAIL_FORMATTER_EXTENSION(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), E_TYPE_MAIL_FORMATTER_EXTENSION))
#define E_IS_MAIL_FORMATTER_EXTENSION_INTERFACE(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), E_TYPE_MAIL_FORMATTER_EXTENSION))
#define E_MAIL_FORMATTER_EXTENSION_GET_INTERFACE(obj) \
	(G_TYPE_INSTANCE_GET_INTERFACE \
	((obj), E_TYPE_MAIL_FORMATTER_EXTENSION, EMailFormatterExtensionInterface))

#define EMF_EXTENSION_GET_FORMATTER(e) \
	E_MAIL_FORMATTER (e_extension_get_extensible (E_EXTENSION (e)))

G_BEGIN_DECLS

typedef struct _EMailFormatterExtension EMailFormatterExtension;
typedef struct _EMailFormatterExtensionInterface EMailFormatterExtensionInterface;

struct _EMailFormatterExtensionInterface {
	GTypeInterface parent_interface;

	/* This is a NULL-terminated array of supported MIME types.
	 * The MIME types can be exact (e.g. "text/plain") or use a
	 * wildcard (e.g. "text/ *"). */
	const gchar **mime_types;

	gboolean	(*format)	(EMailFormatterExtension *extension,
					 EMailFormatter *formatter,
					 EMailFormatterContext *context,
					 EMailPart *part,
					 CamelStream *stream,
					 GCancellable *cancellable);

	GtkWidget *	(*get_widget)	(EMailFormatterExtension *extension,
					 EMailPartList *context,
					 EMailPart *part,
					 GHashTable *params);

	const gchar *	(*get_display_name)
					(EMailFormatterExtension *extension);

	const gchar *	(*get_description)
					(EMailFormatterExtension *extension);

};

GType		e_mail_formatter_extension_get_type
						(void);

gboolean	e_mail_formatter_extension_format
						(EMailFormatterExtension *extension,
						 EMailFormatter *formatter,
						 EMailFormatterContext *context,
						 EMailPart *part,
						 CamelStream *stream,
						 GCancellable *cancellable);

gboolean	e_mail_formatter_extension_has_widget
						(EMailFormatterExtension *extension);

GtkWidget *	e_mail_formatter_extension_get_widget
						(EMailFormatterExtension *extension,
						 EMailPartList *context,
						 EMailPart *part,
						 GHashTable *params);

const gchar *	e_mail_formatter_extension_get_display_name
						(EMailFormatterExtension *extension);

const gchar *	e_mail_formatter_extension_get_description
						(EMailFormatterExtension *extension);

G_END_DECLS

#endif /* E_MAIL_FORMATTER_EXTENSION_H */
