/*
 * e-mail-part-attachment.c
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

#include "e-mail-part-attachment.h"

#define E_MAIL_PART_ATTACHMENT_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), E_TYPE_MAIL_PART_ATTACHMENT, EMailPartAttachmentPrivate))

struct _EMailPartAttachmentPrivate {
	EAttachment *attachment;
};

enum {
	PROP_0,
	PROP_ATTACHMENT
};

G_DEFINE_TYPE (
	EMailPartAttachment,
	e_mail_part_attachment,
	E_TYPE_MAIL_PART)

static void
mail_part_attachment_get_property (GObject *object,
                                   guint property_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_ATTACHMENT:
			g_value_take_object (
				value,
				e_mail_part_attachment_ref_attachment (
				E_MAIL_PART_ATTACHMENT (object)));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
mail_part_attachment_dispose (GObject *object)
{
	EMailPartAttachmentPrivate *priv;

	priv = E_MAIL_PART_ATTACHMENT_GET_PRIVATE (object);

	g_clear_object (&priv->attachment);

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (e_mail_part_attachment_parent_class)->
		dispose (object);
}

static void
mail_part_attachment_finalize (GObject *object)
{
	EMailPartAttachment *part = E_MAIL_PART_ATTACHMENT (object);

	g_free (part->attachment_view_part_id);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (e_mail_part_attachment_parent_class)->
		finalize (object);
}

static void
mail_part_attachment_constructed (GObject *object)
{
	EMailPartAttachmentPrivate *priv;
	CamelMimePart *mime_part;
	EAttachment *attachment;
	EMailPart *part;
	const gchar *cid;

	part = E_MAIL_PART (object);
	priv = E_MAIL_PART_ATTACHMENT_GET_PRIVATE (object);

	/* Chain up to parent's constructed() method. */
	G_OBJECT_CLASS (e_mail_part_attachment_parent_class)->
		constructed (object);

	e_mail_part_set_mime_type (part, E_MAIL_PART_ATTACHMENT_MIME_TYPE);
	e_mail_part_set_is_attachment (part, TRUE);

	mime_part = e_mail_part_ref_mime_part (part);

	cid = camel_mime_part_get_content_id (mime_part);
	if (cid != NULL) {
		gchar *cid_uri;

		cid_uri = g_strconcat ("cid:", cid, NULL);
		e_mail_part_set_cid (part, cid_uri);
		g_free (cid_uri);
	}

	attachment = e_attachment_new ();
	e_attachment_set_mime_part (attachment, mime_part);
	priv->attachment = g_object_ref (attachment);
	g_object_unref (attachment);

	g_object_unref (mime_part);
}

static void
e_mail_part_attachment_class_init (EMailPartAttachmentClass *class)
{
	GObjectClass *object_class;

	g_type_class_add_private (class, sizeof (EMailPartAttachmentPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->get_property = mail_part_attachment_get_property;
	object_class->dispose = mail_part_attachment_dispose;
	object_class->finalize = mail_part_attachment_finalize;
	object_class->constructed = mail_part_attachment_constructed;

	g_object_class_install_property (
		object_class,
		PROP_ATTACHMENT,
		g_param_spec_object (
			"attachment",
			"Attachment",
			"The attachment object",
			E_TYPE_ATTACHMENT,
			G_PARAM_READABLE |
			G_PARAM_STATIC_STRINGS));
}

static void
e_mail_part_attachment_init (EMailPartAttachment *part)
{
	part->priv = E_MAIL_PART_ATTACHMENT_GET_PRIVATE (part);
}

EMailPartAttachment *
e_mail_part_attachment_new (CamelMimePart *mime_part,
                            const gchar *id)
{
	g_return_val_if_fail (id != NULL, NULL);

	return g_object_new (
		E_TYPE_MAIL_PART_ATTACHMENT,
		"id", id, "mime-part", mime_part, NULL);
}

EAttachment *
e_mail_part_attachment_ref_attachment (EMailPartAttachment *part)
{
	g_return_val_if_fail (E_IS_MAIL_PART_ATTACHMENT (part), NULL);

	return g_object_ref (part->priv->attachment);
}

