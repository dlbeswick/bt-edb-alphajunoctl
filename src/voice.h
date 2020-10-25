#pragma once

#include <gst/gst.h>
#include "libbuzztrax-gst/musicenums.h"

G_BEGIN_DECLS
G_DECLARE_FINAL_TYPE(GstBtAlphaJunoCtlV, gstbt_alphajunoctlv, GSTBT, ALPHAJUNOCTLV, GstObject);

GstBtAlphaJunoCtlV* gstbt_alphajunoctlv_new(int channel);

G_END_DECLS
