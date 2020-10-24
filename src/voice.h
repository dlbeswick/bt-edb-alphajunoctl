#pragma once

#include <gst/gst.h>
#include "libbuzztrax-gst/musicenums.h"

G_BEGIN_DECLS
G_DECLARE_FINAL_TYPE(GstBtAlphaJunoCtlV, gstbt_alphajunoctlv, GSTBT, ALPHAJUNOCTLV, GstObject)
//#define GSTBT_ALPHAJUNOCTLV(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),gstbt_alphajunoctlv_get_type(),GstBtAlphaJunoCtlV))

G_END_DECLS
