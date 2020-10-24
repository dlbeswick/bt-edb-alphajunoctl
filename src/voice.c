#include "config.h"
#include "libbuzztrax-gst/propertymeta.h"
#include "libbuzztrax-gst/tempo.h"

#include "src/voice.h"

struct _GstBtAlphaJunoCtlV
{
	GstObject parent;
	
	GstBtNote note;
	guint velocity;
	guint modulation;
	gboolean hold;
	guint aftertouch;
};


enum
{
	PROP_NOTE = 1,
	PROP_VELOCITY,
	PROP_MODULATION,
	PROP_HOLD,
	PROP_AFTERTOUCH
};


G_DEFINE_TYPE (GstBtAlphaJunoCtlV, gstbt_alphajunoctlv, GST_TYPE_OBJECT);

//-- property meta interface implementations

static void
gst_alphajunoctlv_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
	GstBtAlphaJunoCtlV *src = GSTBT_ALPHAJUNOCTLV(object);

	switch (prop_id)
	{
    case PROP_NOTE:
	{
		guint note = g_value_get_enum (value);
		GST_INFO_OBJECT (src, "note: %d", note);
		if (note) {
			src->note = (GstBtNote) note;
		}
    }
	break;
	
	case PROP_VELOCITY:
		break;
	case PROP_MODULATION:
		break;
	case PROP_HOLD:
		break;
	case PROP_AFTERTOUCH:
		break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_alphajunoctlv_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
	GstBtAlphaJunoCtlV *src = GSTBT_ALPHAJUNOCTLV (object);

	switch (prop_id)
	{
	case PROP_VELOCITY:
		break;
	case PROP_MODULATION:
		break;
	case PROP_HOLD:
		break;
	case PROP_AFTERTOUCH:
		break;
    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gstbt_alphajunoctlv_init (GstBtAlphaJunoCtlV * self)
{
}

static void
gstbt_alphajunoctlv_class_init (GstBtAlphaJunoCtlVClass * klass)
{
  GObjectClass *const gobject_class = G_OBJECT_CLASS (klass);
  const GParamFlags flags = (GParamFlags)
      (G_PARAM_WRITABLE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS);

  gobject_class->set_property = gst_alphajunoctlv_set_property;
  gobject_class->get_property = gst_alphajunoctlv_get_property;

  g_object_class_install_property(
	  gobject_class, PROP_NOTE,
      g_param_spec_enum ("note", "Musical note", "", GSTBT_TYPE_NOTE, GSTBT_NOTE_NONE, flags));

  g_object_class_install_property(
	  gobject_class, PROP_VELOCITY,
	  g_param_spec_uint ("velocity", "Velocity", "", 0, 0x7F, 0x7F, flags));

  g_object_class_install_property(
	  gobject_class, PROP_MODULATION,
      g_param_spec_uint ("modulation", "Modulation", "", 0, 0x7F, 0, flags));

  g_object_class_install_property(
	  gobject_class, PROP_HOLD,
      g_param_spec_boolean ("hold", "Hold", "", FALSE, flags));

  g_object_class_install_property(
	  gobject_class, PROP_AFTERTOUCH,
	  g_param_spec_uint ("aftertouch", "Aftertouch", "", 0, 0x7F, 0x7F, flags));
}
