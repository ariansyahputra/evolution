/*
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
 *
 * Authors:
 *   Jeffrey Stedfast <fejj@ximian.com>
 *   Radek Doulik <rodo@ximian.com>
 *   Jonathon Jongsma <jonathon.jongsma@collabora.co.uk>
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 * Copyright (C) 2009 Intel Corporation
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libedataserver/e-data-server-util.h>
#include <e-util/e-util.h>
#include "e-util/e-account-utils.h"
#include "e-util/e-signature-utils.h"

#include "e-mail-local.h"
#include "e-mail-folder-utils.h"
#include "mail-config.h"
#include "mail-tools.h"

typedef struct {
	GSList *labels;

	gboolean address_compress;
	gint address_count;

	GSList *jh_header;
	gboolean jh_check;
	gboolean book_lookup;
	gboolean book_lookup_local_only;
} MailConfig;

extern gint camel_header_param_encode_filenames_in_rfc_2047;

static MailConfig *config = NULL;
static GSettings *mail_settings = NULL;

static void
settings_outlook_filenames_changed (GSettings *settings,
				    const gchar *key,
				    gpointer user_data)
{
	/* pass option to the camel */
	if (g_settings_get_boolean (settings, key))
		camel_header_param_encode_filenames_in_rfc_2047 = 1;
	else
		camel_header_param_encode_filenames_in_rfc_2047 = 0;
}

static void
settings_jh_headers_changed (GSettings *settings,
			     const gchar *key,
			     EMailSession *session)
{
	GSList *node;
	GPtrArray *name, *value;
	gchar **strv;
	gint i;

	g_slist_foreach (config->jh_header, (GFunc) g_free, NULL);
	g_slist_free (config->jh_header);
	config->jh_header = NULL;

	strv = g_settings_get_strv (settings, "junk-custom-header");
	for (i = 0; strv[i] != NULL; i++)
		config->jh_header = g_slist_append (config->jh_header, g_strdup (strv[i]));
	g_strfreev (strv);

	node = config->jh_header;
	name = g_ptr_array_new ();
	value = g_ptr_array_new ();
	while (node && node->data) {
		gchar **tok = g_strsplit (node->data, "=", 2);
		g_ptr_array_add (name, g_strdup (tok[0]));
		g_ptr_array_add (value, g_strdup (tok[1]));
		node = node->next;
		g_strfreev (tok);
	}
	camel_session_set_junk_headers (
		CAMEL_SESSION (session),
		(const gchar **) name->pdata,
		(const gchar **) value->pdata, name->len);

	g_ptr_array_foreach (name, (GFunc) g_free, NULL);
	g_ptr_array_foreach (value, (GFunc) g_free, NULL);
	g_ptr_array_free (name, TRUE);
	g_ptr_array_free (value, TRUE);
}

static void
settings_jh_check_changed (GSettings *settings,
			   const gchar *key,
			   EMailSession *session)
{
	config->jh_check = g_settings_get_boolean (settings, "junk-check-custom-header");
	if (!config->jh_check) {
		camel_session_set_junk_headers (
			CAMEL_SESSION (session), NULL, NULL, 0);
	} else {
		settings_jh_headers_changed (settings, NULL, session);
	}
}

static void
settings_bool_value_changed (GSettings *settings,
			     const gchar *key,
			     gboolean *save_location)
{
	*save_location = g_settings_get_boolean (settings, key);
}

static void
settings_int_value_changed (GSettings *settings,
			    const gchar *key,
			    gint *save_location)
{
	*save_location = g_settings_get_int (settings, key);
}

void
mail_config_write (void)
{
	EAccountList *account_list;
	ESignatureList *signature_list;

	if (!config)
		return;

	account_list = e_get_account_list ();
	signature_list = e_get_signature_list ();

	e_account_list_save (account_list);
	e_signature_list_save (signature_list);

	g_settings_sync ();
}

gint
mail_config_get_address_count (void)
{
	if (!config->address_compress)
		return -1;

	return config->address_count;
}

/* timeout interval, in seconds, when to call server update */
gint
mail_config_get_sync_timeout (void)
{
	gint res = 60;

	res = g_settings_get_int (mail_settings, "sync-interval");

	/* do not allow recheck sooner than every 30 seconds */
	if (res == 0)
		res = 60;
	else if (res < 30)
		res = 30;

	return res;
}

gchar *
mail_config_folder_to_cachename (CamelFolder *folder,
                                 const gchar *prefix)
{
	gchar *folder_uri, *basename, *filename;
	const gchar *config_dir;

	config_dir = mail_session_get_config_dir ();

	basename = g_build_filename (config_dir, "folders", NULL);
	if (!g_file_test (basename, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
		/* create the folder if does not exist */
		g_mkdir_with_parents (basename, 0700);
	}
	g_free (basename);

	folder_uri = e_mail_folder_uri_from_folder (folder);
	e_filename_make_safe (folder_uri);
	basename = g_strdup_printf ("%s%s", prefix, folder_uri);
	filename = g_build_filename (config_dir, "folders", basename, NULL);
	g_free (basename);
	g_free (folder_uri);

	return filename;
}

void
mail_config_reload_junk_headers (EMailSession *session)
{
	g_return_if_fail (E_IS_MAIL_SESSION (session));

	/* It automatically sets in the session */
	if (config == NULL)
		mail_config_init (session);
	else {
		settings_jh_check_changed (mail_settings, NULL, session);
	}
}

gboolean
mail_config_get_lookup_book (void)
{
	g_return_val_if_fail (config != NULL, FALSE);

	return config->book_lookup;
}

gboolean
mail_config_get_lookup_book_local_only (void)
{
	g_return_val_if_fail (config != NULL, FALSE);

	return config->book_lookup_local_only;
}

/* Config struct routines */
void
mail_config_init (EMailSession *session)
{
	g_return_if_fail (E_IS_MAIL_SESSION (session));

	if (config)
		return;

	config = g_new0 (MailConfig, 1);

	mail_settings = g_settings_new ("org.gnome.evolution.mail");

	/* Composer Configuration */

	settings_outlook_filenames_changed (
		mail_settings, "composer-outlook-filenames", NULL);
	g_signal_connect (
		mail_settings, "changed::composer-outlook-filenames",
		G_CALLBACK (settings_outlook_filenames_changed), NULL);

	/* Display Configuration */

	g_signal_connect (
		mail_settings, "changed::address-compress",
		G_CALLBACK (settings_bool_value_changed), &config->address_compress);
	config->address_compress = g_settings_get_boolean (mail_settings, "address-compress");

	g_signal_connect (
		mail_settings, "changed::address-count",
		G_CALLBACK (settings_int_value_changed), &config->address_count);
	config->address_count = g_settings_get_int (mail_settings, "address-count");

	/* Junk Configuration */

	g_signal_connect (
		mail_settings, "changed::junk-check-custom-header",
		G_CALLBACK (settings_jh_check_changed), session);
	config->jh_check = g_settings_get_boolean (mail_settings, "junk-check-custom-header");

	g_signal_connect (
		mail_settings, "changed::junk-custom-header",
		G_CALLBACK (settings_jh_headers_changed), session);

	g_signal_connect (
		mail_settings, "changed::junk-lookup-addressbook",
		G_CALLBACK (settings_bool_value_changed), &config->book_lookup);
	config->book_lookup = g_settings_get_boolean (mail_settings, "junk-lookup-addressbook");

	g_signal_connect (
		mail_settings, "changed::junk-lookup-addressbook-local-only",
		G_CALLBACK (settings_bool_value_changed), &config->book_lookup_local_only);
	config->book_lookup_local_only = g_settings_get_boolean (mail_settings, "junk-lookup-addressbook-local-only");

	settings_jh_check_changed (mail_settings, NULL, session);
}
