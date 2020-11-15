#pragma once

#include <gst/gst.h>

G_BEGIN_DECLS
G_DECLARE_FINAL_TYPE(GstBtAlphaJunoCtlV, gstbt_alphajunoctlv, GSTBT, ALPHAJUNOCTLV, GstObject)

GstBtAlphaJunoCtlV* gstbt_alphajunoctlv_new(int channel);
void gstbt_alphajunoctlv_process(GstBtAlphaJunoCtlV* self, GstBuffer* gstbuf);
void gstbt_alphajunoctlv_noteall_off(GstBtAlphaJunoCtlV* self);

G_END_DECLS
