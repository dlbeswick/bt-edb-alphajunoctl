#pragma once

#include <glib-object.h>

G_BEGIN_DECLS
#define GSTBT_ALPHAJUNOCTL(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),gstbt_alphajunoctl_get_type(),GstBtAlphaJunoCtl))

GType gstbt_alphajunoctl_get_type(void);

G_END_DECLS
