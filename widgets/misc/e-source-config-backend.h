/*
 * e-source-config-backend.h
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

#ifndef E_SOURCE_CONFIG_BACKEND_H
#define E_SOURCE_CONFIG_BACKEND_H

#include <libebackend/e-extension.h>

#include <misc/e-source-config.h>

/* Standard GObject macros */
#define E_TYPE_SOURCE_CONFIG_BACKEND \
	(e_source_config_backend_get_type ())
#define E_SOURCE_CONFIG_BACKEND(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), E_TYPE_SOURCE_CONFIG_BACKEND, ESourceConfigBackend))
#define E_SOURCE_CONFIG_BACKEND_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), E_TYPE_SOURCE_CONFIG_BACKEND, ESourceConfigBackendClass))
#define E_IS_SOURCE_CONFIG_BACKEND(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), E_TYPE_SOURCE_CONFIG_BACKEND))
#define E_IS_SOURCE_CONFIG_BACKEND_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), E_TYPE_SOURCE_CONFIG_BACKEND))
#define E_SOURCE_CONFIG_BACKEND_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), E_TYPE_SOURCE_CONFIG_BACKEND, ESourceConfigBackendClass))

G_BEGIN_DECLS

typedef struct _ESourceConfigBackend ESourceConfigBackend;
typedef struct _ESourceConfigBackendClass ESourceConfigBackendClass;
typedef struct _ESourceConfigBackendPrivate ESourceConfigBackendPrivate;

struct _ESourceConfigBackend {
	EExtension parent;
	ESourceConfigBackendPrivate *priv;
};

struct _ESourceConfigBackendClass {
	EExtensionClass parent_class;

	const gchar *parent_uid;
	const gchar *backend_name;

	gboolean	(*allow_creation)	(ESourceConfigBackend *backend);
	void		(*insert_widgets)	(ESourceConfigBackend *backend,
						 ESource *scratch_source);
	gboolean	(*check_complete)	(ESourceConfigBackend *backend,
						 ESource *scratch_source);
	void		(*commit_changes)	(ESourceConfigBackend *backend,
						 ESource *scratch_source);
};

GType		e_source_config_backend_get_type
					(void) G_GNUC_CONST;
ESourceConfig *	e_source_config_backend_get_config
					(ESourceConfigBackend *backend);
gboolean	e_source_config_backend_allow_creation
					(ESourceConfigBackend *backend);
void		e_source_config_backend_insert_widgets
					(ESourceConfigBackend *backend,
					 ESource *scratch_source);
gboolean	e_source_config_backend_check_complete
					(ESourceConfigBackend *backend,
					 ESource *scratch_source);
void		e_source_config_backend_commit_changes
					(ESourceConfigBackend *backend,
					 ESource *scratch_source);

G_END_DECLS

#endif /* E_SOURCE_CONFIG_BACKEND_H */
