/* GTK - The GIMP Toolkit
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "gtkintl.h"
#include "gtk/gtkimmodule.h"
#include "gtkimcontextgcin.h"
#include <string.h>

static const GtkIMContextInfo gcin_info = {
  "gcin",		           /* ID */
  N_("gcin Input Method"),            /* Human readable name */
  GETTEXT_PACKAGE,		   /* Translation domain */
  GTK_LOCALEDIR,		   /* Dir for bindtextdomain (not strictly needed for "gtk+") */
  "zh:ja"		           /* Languages for which this module is the default */
//  "zh_TW"		           /* Languages for which this module is the default */
};

static const GtkIMContextInfo *info_list[] = {
  &gcin_info
};

void
im_module_init (GTypeModule *type_module)
{
//  printf("im_module_init\n");
  gtk_im_context_gcin_register_type (type_module);
}

void
im_module_exit (void)
{
//  printf("im_module_exit\n");
  gtk_im_context_gcin_shutdown ();
}

void
im_module_list (const GtkIMContextInfo ***contexts,
		int                      *n_contexts)
{
//  printf("im_module_list\n");
  *contexts = info_list;
  *n_contexts = G_N_ELEMENTS (info_list);
}

GtkIMContext *
im_module_create (const gchar *context_id)
{
//  printf("im_module_create %s\n", context_id);
  if (strcmp (context_id, "gcin") == 0)
    return gtk_im_context_gcin_new ();
  else
    return NULL;
}
