/*
 * e-mail-formatter-quote-text-enriched.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "e-mail-format-extensions.h"

#include <em-format/e-mail-formatter-extension.h>
#include <em-format/e-mail-formatter.h>
#include <em-format/e-mail-inline-filter.h>
#include <e-util/e-util.h>

#include <glib/gi18n-lib.h>
#include <camel/camel.h>

typedef struct _EMailFormatterQuoteTextEnriched {
	GObject parent;
} EMailFormatterQuoteTextEnriched;

typedef struct _EMailFormatterQuoteTextEnrichedClass {
	GObjectClass parent_class;
} EMailFormatterQuoteTextEnrichedClass;

static void e_mail_formatter_quote_formatter_extension_interace_init
					(EMailFormatterExtensionInterface *iface);

G_DEFINE_TYPE_EXTENDED (
	EMailFormatterQuoteTextEnriched,
	e_mail_formatter_quote_text_enriched,
	G_TYPE_OBJECT,
	0,
	G_IMPLEMENT_INTERFACE (
		E_TYPE_MAIL_FORMATTER_EXTENSION,
		e_mail_formatter_quote_formatter_extension_interace_init));

static const gchar *formatter_mime_types[] = {
	"text/enriched",
	"text/richtext",
	NULL
};

static gboolean
emqfe_text_enriched_format (EMailFormatterExtension *extension,
                            EMailFormatter *formatter,
                            EMailFormatterContext *context,
                            EMailPart *part,
                            CamelStream *stream,
                            GCancellable *cancellable)
{
	CamelStream *filtered_stream;
	CamelMimeFilter *enriched;
	guint32 camel_flags = 0;

	if (g_strcmp0 (part->mime_type, "text/richtext") == 0) {
		camel_flags = CAMEL_MIME_FILTER_ENRICHED_IS_RICHTEXT;
		camel_stream_write_string (
			stream, "\n<!-- text/richtext -->\n",
			cancellable, NULL);
	} else {
		camel_stream_write_string (
			stream, "\n<!-- text/enriched -->\n",
			cancellable, NULL);
	}

	enriched = camel_mime_filter_enriched_new (camel_flags);
	filtered_stream = camel_stream_filter_new (stream);
	camel_stream_filter_add (
		CAMEL_STREAM_FILTER (filtered_stream), enriched);
	g_object_unref (enriched);

	camel_stream_write_string (stream, "<br><hr><br>", cancellable, NULL);
	e_mail_formatter_format_text (formatter, part, filtered_stream, cancellable);
	camel_stream_flush (filtered_stream, cancellable, NULL);
	g_object_unref (filtered_stream);

	return TRUE;
}

static const gchar *
emqfe_text_enriched_get_display_name (EMailFormatterExtension *extension)
{
	return _("Richtext");
}

static const gchar *
emqfe_text_enriched_get_description (EMailFormatterExtension *extension)
{
	return _("Display part as enriched text");
}

static void
e_mail_formatter_quote_text_enriched_class_init (EMailFormatterQuoteTextEnrichedClass *class)
{
}

static void
e_mail_formatter_quote_formatter_extension_interace_init (EMailFormatterExtensionInterface *iface)
{
	iface->mime_types = formatter_mime_types;
	iface->format = emqfe_text_enriched_format;
	iface->get_display_name = emqfe_text_enriched_get_display_name;
	iface->get_description = emqfe_text_enriched_get_description;
}

static void
e_mail_formatter_quote_text_enriched_init (EMailFormatterQuoteTextEnriched *formatter)
{

}
