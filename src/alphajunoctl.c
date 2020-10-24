#include "config.h"
#include "alphajunoctl.h"
#include "genums.h"
#include "generated/generated-genums.h"

#include "libbuzztrax-gst/childbin.h"
#include "libbuzztrax-gst/propertymeta.h"
#include <stdio.h>

#define GST_MACHINE_NAME "alphajunoctl"
#define GST_MACHINE_DESC "Controller for real Alpha Juno synth via MIDI"

GST_DEBUG_CATEGORY (alphajunoctl_debug);

/*
http://www.vintagesynth.com/roland/sysex.php

00 	DCO Env. Mode (0=Normal, 1=Inverted, 2=Normal-Dynamic, 3=Inv.-Dynamic) 	F0 41 36 00 23 20 01 00 00 F7
01 	VCF Env. Mode (0=Normal, 1=Inverted, 2=Normal-Dynamic, 3=Dynamic) 	F0 41 36 00 23 20 01 01 00 F7
02 	VCA Env. Mode (0=Normal, 1=Gate, 2=Normal-Dynamic, 3=Gate-Dynamic) 	F0 41 36 00 23 20 01 02 00 F7
03 	DCO Wave Pulse (0..3) 	F0 41 36 00 23 20 01 03 00 F7
04 	DCO Wave Saw (0..5) 	F0 41 36 00 23 20 01 04 00 F7
05 	DCO Wave Sub (0..5) 	F0 41 36 00 23 20 01 05 00 F7
06 	DCO Range (0=4', 1=8', 2=16', 3=32') 	F0 41 36 00 23 20 01 06 00 F7
07 	DCO Sub Level (0..3) 	F0 41 36 00 23 20 01 07 00 F7
08 	DCO Noise (0..3) 	F0 41 36 00 23 20 01 08 00 F7
09 	HPF Cutoff (0..3) 	F0 41 36 00 23 20 01 09 00 F7
0A 	Chorus Switch (0=Off, 1=On) 	F0 41 36 00 23 20 01 0A 00 F7
0B 	DCO LFO Mod. (0..7F) 	F0 41 36 00 23 20 01 0B 00 F7
0C 	DCO ENV Mod. (0..7F) 	F0 41 36 00 23 20 01 0C 00 F7
0D 	DCO After Mod. (0..7F) 	F0 41 36 00 23 20 01 0D 00 F7
0E 	DCO PWM Depth (0..7F) 	F0 41 36 00 23 20 01 0E 00 F7
0F 	DCO PWM Rate (0..7F) 0 = Pulse Width Manual 1..7F = PW LFO Rate 	F0 41 36 00 23 20 01 0F 00 F7
10 	VCF Cutoff (0..7F) 	F0 41 36 00 23 20 01 10 00 F7
11 	VCF Resonance (0..7F) 	F0 41 36 00 23 20 01 11 00 F7
12 	VCF LFO Mod. (0..7F) 	F0 41 36 00 23 20 01 12 00 F7
13 	VCF ENV Mod. (0..7F) 	F0 41 36 00 23 20 01 13 00 F7
14 	VCF Key Follow (0..7F) 	F0 41 36 00 23 20 01 14 00 F7
15 	VCF Aftertouch (0..7F) 	F0 41 36 00 23 20 01 15 00 F7
16 	VCA Level (0..7F) 	F0 41 36 00 23 20 01 16 00 F7
17 	VCA Aftertouch (0..7F) 	F0 41 36 00 23 20 01 17 00 F7
18 	LFO Rate (0..7F) 	F0 41 36 00 23 20 01 18 00 F7
19 	LFO Delay (0..7F) 	F0 41 36 00 23 20 01 19 00 F7
1A 	ENV T1 (0..7F) Attack Time 	F0 41 36 00 23 20 01 1A 00 F7
1B 	ENV L1 (0..7F) Attack Level 	F0 41 36 00 23 20 01 1B 00 F7
1C 	ENV T2 (0..7F) Break Time 	F0 41 36 00 23 20 01 1C 00 F7
1D 	ENV L2 (0..7F) Break Level 	F0 41 36 00 23 20 01 1D 00 F7
1E 	ENV T3 (0..7F) Decay Time 	F0 41 36 00 23 20 01 1E 00 F7
1F 	ENV L3 (0..7F) Sustain Level 	F0 41 36 00 23 20 01 1F 00 F7
20 	ENV T4 (0..7F) Release Time 	F0 41 36 00 23 20 01 20 00 F7
21 	ENV Key Follow (0..7F) 	F0 41 36 00 23 20 01 21 00 F7
22 	Chorus Rate (0..7F) 	F0 41 36 00 23 20 01 22 00 F7
23 	Bender Range (0..C) 	F0 41 36 00 23 20 01 23 00 F7
*/


enum
{
	// static class properties
	PROP_CHILDREN = 1,
	PROP_BENDERRANGE,
	PROP_CHORUS,
	PROP_CHORUSRATE,
	PROP_DCORANGE,
	PROP_DCONOISE,
	PROP_DCOAFTERTOUCH,
	PROP_DCOLFOMOD,
	PROP_DCOENVMODE,
	PROP_DCOENVMOD,
	PROP_DCOPWMDEPTH,
	PROP_DCOPWMRATE,
	PROP_DCOWAVEPULSE,
	PROP_DCOWAVESAW,
	PROP_DCOWAVESUB,
	PROP_DCOSUBLEVEL,
	PROP_ENVT1ATTACKTIME,
	PROP_ENVT1ATTACKLEVEL,
	PROP_ENVT2BREAKTIME,
	PROP_ENVT2BREAKLEVEL,
	PROP_ENVT3DECAYTIME,
	PROP_ENVT3SUSTAINLEVEL,
	PROP_ENVT4RELEASETIME,
	PROP_ENVKEYFOLLOW,
	PROP_HPFCUTOFF,
	PROP_LFORATE,
	PROP_LFODELAY,
	PROP_VCFAFTERTOUCH,
	PROP_VCFCUTOFF,
	PROP_VCFRESONANCE,
	PROP_VCFENVMODE,
	PROP_VCFKEYFOLLOW,
	PROP_VCFLFOMOD,
	PROP_VCFENVMOD,
	PROP_VCAAFTERTOUCH,
	PROP_VCAENVMODE,
	PROP_VCALEVEL,

	PROP_PITCHBEND,
	PROP_PORTAMENTO,
	PROP_PORTAMENTOTIME,
	PROP_POLYMODE,
	PROP_MAINVOLUME,
	PROP_TUNING,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

static GObject *gstbt_alphajunoctl_child_proxy_get_child_by_index (GstChildProxy *child_proxy, guint index) {
  GstBtAlphaJunoCtl *self = GSTBT_ALPHAJUNOCTL(child_proxy);

  g_return_val_if_fail(index < MAX_VOICES, NULL);

  return (GObject *)gst_object_ref(self->voices[index]);
}

static guint gstbt_alphajunoctl_child_proxy_get_children_count (GstChildProxy *child_proxy) {
  GstBtAlphaJunoCtl *self = GSTBT_ALPHAJUNOCTL(child_proxy);
  return self->cntVoices;
}

static void gstbt_alphajunoctl_child_proxy_interface_init (gpointer g_iface, gpointer iface_data) {
  GstChildProxyInterface *iface = (GstChildProxyInterface *)g_iface;

  GST_INFO("initializing iface");

  iface->get_child_by_index = gstbt_alphajunoctl_child_proxy_get_child_by_index;
  iface->get_children_count = gstbt_alphajunoctl_child_proxy_get_children_count;
}


//-- the class
G_DEFINE_TYPE_WITH_CODE (
	GstBtAlphaJunoCtl,
	gstbt_alphajunoctl,
	GSTBT_TYPE_AUDIO_SYNTH,
	G_IMPLEMENT_INTERFACE (GST_TYPE_CHILD_PROXY, gstbt_alphajunoctl_child_proxy_interface_init)
	G_IMPLEMENT_INTERFACE (GSTBT_TYPE_CHILD_BIN, NULL));

static void _set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstBtAlphaJunoCtl *src = GSTBT_ALPHAJUNOCTL (object);

  switch (prop_id) {
    case PROP_CHILDREN:
      src->cntVoices = g_value_get_ulong(value);
      break;
    case PROP_BENDERRANGE:
		src->benderRange = g_value_get_uint(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void _get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstBtAlphaJunoCtl *src = GSTBT_ALPHAJUNOCTL (object);

  switch (prop_id) {
    case PROP_CHILDREN:
      g_value_set_ulong (value, src->cntVoices);
      break;
    case PROP_BENDERRANGE:
		g_value_set_uint(value, src->benderRange);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void _setup (GstBtAudioSynth * base, GstAudioInfo * info)
{
}

static void _dispose(GObject * object)
{
  GstBtAlphaJunoCtl *src = GSTBT_ALPHAJUNOCTL(object);
  G_OBJECT_CLASS(gstbt_alphajunoctl_parent_class)->dispose(object);
}

static void gstbt_alphajunoctl_class_init (GstBtAlphaJunoCtlClass * klass)
{
	GObjectClass* const gobject_class = (GObjectClass *) klass;
	gobject_class->set_property = _set_property;
	gobject_class->get_property = _get_property;
	gobject_class->dispose = _dispose;

	GstElementClass* const element_class = (GstElementClass *) klass;
	gst_element_class_set_static_metadata(
		element_class,
		"Alpha Juno Control",
		"Source/Audio",
		GST_MACHINE_DESC,
		"David Beswick <" PACKAGE_BUGREPORT ">");

	GstBtAudioSynthClass* const audio_synth_class = (GstBtAudioSynthClass *) klass;
	audio_synth_class->setup = _setup;
  
/*  audio_synth_class->process = gstbt_sim_syn_process;
  audio_synth_class->reset = gstbt_sim_syn_reset;
  audio_synth_class->negotiate = gstbt_sim_syn_negotiate;

  // TBD: docs
/*  gst_element_class_add_metadata (element_class, GST_ELEMENT_METADATA_DOC_URI,
      "file://" DATADIR "" G_DIR_SEPARATOR_S "gtk-doc" G_DIR_SEPARATOR_S "html"
      G_DIR_SEPARATOR_S "" PACKAGE "-gst" G_DIR_SEPARATOR_S "GstBtSimSyn.html");*/

	const GParamFlags flags =
		(GParamFlags)(G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS);
  
	// GstBtChildBin interface properties
	properties[PROP_CHILDREN] = g_param_spec_ulong(
		"children",
		"",
		"",
		1, MAX_VOICES, 1,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	// Instance properties
	properties[PROP_BENDERRANGE] =
		g_param_spec_uint("bender-range", "Bender Range", "", 0, 13, 5, flags);
	properties[PROP_CHORUS] =
		g_param_spec_uint("chorus", "Chorus", "", 0, 1, 0, flags);
	properties[PROP_CHORUSRATE] =
		g_param_spec_uint("chorus-rate", "Chorus Rate", "", 0, 127, 44, flags);
	properties[PROP_DCORANGE] =
		g_param_spec_enum("dco-range", "DCO Range", "", gst_alpha_juno_ctl_dco_range_get_type(),
						  DCORANGE_32, flags);
	properties[PROP_DCOENVMODE] =
		g_param_spec_enum("dco-envmode", "DCO Env Mode", "", gst_alpha_juno_ctl_dco_env_mode_get_type(),
						  DCOENVMODE_NORMALDYNAMIC, flags);
	properties[PROP_DCONOISE] =
		g_param_spec_uint("dco-noise", "DCO Noise", "", 0x00, 0x03, 1, flags);
	properties[PROP_DCOAFTERTOUCH] =
		g_param_spec_uint("dco-aftertouch", "DCO Aftertouch", "", 0x00, 0x7F, 9, flags);
	properties[PROP_DCOLFOMOD] =
		g_param_spec_uint("dco-lfo-mod", "DCO LFO Mod", "", 0x00, 0x7F, 0, flags);
	properties[PROP_DCOENVMOD] =
		g_param_spec_uint("dco-env-mod", "DCO Env Mod", "", 0x00, 0x7F, 127, flags);
	properties[PROP_DCOPWMDEPTH] =
		g_param_spec_uint("pwm-depth", "DCO PWM Depth", "", 0x00, 0x7F, 127, flags);
	properties[PROP_DCOPWMRATE] =
		g_param_spec_uint("pwm-rate", "DCO PWM Rate", "", 0x00, 0x7F, 102, flags);
	properties[PROP_DCOWAVEPULSE] =
		g_param_spec_uint("dco-pulse", "DCO Pulse", "", 0x00, 0x03, 3, flags);
	properties[PROP_DCOWAVESAW] =
		g_param_spec_uint("dco-saw", "DCO Saw", "", 0x00, 0x05, 3, flags);
	properties[PROP_DCOWAVESUB] =
		g_param_spec_uint("dco-sub", "DCO Sub", "", 0x00, 0x05, 5, flags);
	properties[PROP_DCOSUBLEVEL] =
		g_param_spec_uint("sub-level", "DCO Sub Level", "", 0x00, 0x03, 3, flags);
	properties[PROP_ENVT1ATTACKTIME] =
		g_param_spec_uint("t1-time", "ENV T1 Attack Time", "", 0x00, 0x7F, 99, flags);
	properties[PROP_ENVT1ATTACKLEVEL] =
		g_param_spec_uint("t1-Lev", "ENV T1 Attack Level", "", 0x00, 0x7F, 68, flags);
	properties[PROP_ENVT2BREAKTIME] =
		g_param_spec_uint("t2-time", "ENV T2 Break Time", "", 0x00, 0x7F, 78, flags);
	properties[PROP_ENVT2BREAKLEVEL] =
		g_param_spec_uint("t2-lev", "ENV T2 Break Level", "", 0x00, 0x7F, 127, flags);
	properties[PROP_ENVT3DECAYTIME] =
		g_param_spec_uint("t3-time", "ENV T3 Decay Time", "", 0x00, 0x7F, 75, flags);
	properties[PROP_ENVT3SUSTAINLEVEL] =
		g_param_spec_uint("t3-lev", "ENV T3 Sustain Level", "", 0x00, 0x7F, 127, flags);
	properties[PROP_ENVT4RELEASETIME] =
		g_param_spec_uint("t4-time", "ENV T4 Release Time", "", 0x00, 0x7F, 92, flags);
	properties[PROP_ENVKEYFOLLOW] =
		g_param_spec_uint("env-keyfollow", "ENV Key Follow", "", 0x00, 0x7F, 11, flags);
	properties[PROP_HPFCUTOFF] =
		g_param_spec_uint("hpf-cutoff", "HPF Cutoff", "", 0x00, 0x03, 0, flags);
	properties[PROP_LFORATE] =
		g_param_spec_uint("lfo-rate", "LFO Rate", "", 0x00, 0x7F, 39, flags);
	properties[PROP_LFODELAY] =
		g_param_spec_uint("lfo-delay", "LFO Delay", "", 0x00, 0x7F, 64, flags);
	properties[PROP_VCFAFTERTOUCH] =
		g_param_spec_uint("vcf-aftert", "VCF Aftertouch", "", 0x00, 0x7F, 9, flags);
	properties[PROP_VCFCUTOFF] =
		g_param_spec_uint("vcf-cutoff", "VCF Cutoff", "", 0x00, 0x7F, 77, flags);
	properties[PROP_VCFRESONANCE] =
		g_param_spec_uint("vcf-res", "VCF Resonance", "", 0x00, 0x7F, 0, flags);
	properties[PROP_VCFENVMODE] =
		g_param_spec_enum("vcf-envmode", "VCF Env Mode", "", gst_alpha_juno_ctl_vcf_env_mode_get_type(),
						  VCFENVMODE_NORMALDYNAMIC, flags);
	properties[PROP_VCFKEYFOLLOW] =
		g_param_spec_uint("vcf-keyfollow", "VCF Key Follow", "", 0x00, 0x7F, 9, flags);
	properties[PROP_VCFLFOMOD] =
		g_param_spec_uint("vcf-lfo-mod", "VCF LFO Mod", "", 0x00, 0x7F, 75, flags);
	properties[PROP_VCFENVMOD] =
		g_param_spec_uint("vcf-env-mod", "VCF Env Mod", "", 0x00, 0x7F, 75, flags);
	properties[PROP_VCAAFTERTOUCH] =
		g_param_spec_uint("vca-aftert", "VCA Aftertouch", "", 0x00, 0x7F, 0, flags);
	properties[PROP_VCAENVMODE] =
		g_param_spec_enum("dco-vcamode", "VCA Env Mode", "", gst_alpha_juno_ctl_vca_env_mode_get_type(),
						  DCOENVMODE_NORMALDYNAMIC, flags);
	properties[PROP_VCALEVEL] =
		g_param_spec_uint("vca-level", "VCA Level", "", 0x00, 0x7F, 60, flags);
	properties[PROP_PORTAMENTOTIME] =
		g_param_spec_uint("porta-time", "Portamento Time", "", 0x00, 0x7F, 0x00, flags);
	properties[PROP_PORTAMENTO] =
		g_param_spec_boolean("porta", "Portamento", "", FALSE, flags);
	properties[PROP_PITCHBEND] =
		g_param_spec_uint("bend", "Pitch Bend", "", 0x0000, 0x3FFF, 0x2000, flags);
	properties[PROP_POLYMODE] =
		g_param_spec_enum("polymode", "Polyphony mode", "", gst_alpha_juno_ctl_poly_mode_get_type(),
						  POLYMODE_OMNIOFF, flags);
	properties[PROP_MAINVOLUME] =
		g_param_spec_uint("main-vol", "Main Volume", "", 0x00, 0x7F, 0x7F, flags);

  g_object_class_install_properties (gobject_class, N_PROPERTIES, properties);
}

static void
gstbt_alphajunoctl_init (GstBtAlphaJunoCtl * src)
{
	for (int i = 0; i < MAX_VOICES; i++)
	{
		src->voices[i] = (GstBtAlphaJunoCtlV*)g_object_new(gstbt_alphajunoctlv_get_type(), NULL);

		char name[7];
		snprintf(name, sizeof(name), "voice%1d", i);
		
		gst_object_set_name ((GstObject *)src->voices[i],name);
		gst_object_set_parent ((GstObject *)src->voices[i], (GstObject *)src);
		GST_WARNING_OBJECT (src->voices[i], "created %p", src->voices[i]);
	}
}

static void
gstbt_alphajunoctl_dispose (GObject * object)
{
	GstBtAlphaJunoCtl *self = GSTBT_ALPHAJUNOCTL(object);
	gint i;
  
	for (i = 0; i < MAX_VOICES; i++) {
		gst_object_unparent((GstObject *)self->voices[i]);
	}

	G_OBJECT_CLASS(gstbt_alphajunoctl_parent_class)->dispose(object);
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_MACHINE_NAME,
      GST_DEBUG_FG_WHITE | GST_DEBUG_BG_BLACK, GST_MACHINE_DESC);

  return gst_element_register (plugin, GST_MACHINE_NAME, GST_RANK_NONE,
      gstbt_alphajunoctl_get_type());
}

GST_PLUGIN_DEFINE (
	GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    alphajunoctl,
    GST_MACHINE_DESC,
    plugin_init, VERSION, "GPL", PACKAGE_NAME, PACKAGE_BUGREPORT);
