#include "config.h"
#include "src/alphajunoctl.h"
#include "src/genums.h"
#include "src/midi.h"
#include "src/voice.h"
#include "src/generated/generated-genums.h"

#include "libbuzztrax-gst/audiosynth.h"
#include "libbuzztrax-gst/childbin.h"
#include "libbuzztrax-gst/propertymeta.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define GST_MACHINE_NAME "alphajunoctl"
#define GST_MACHINE_DESC "Controller for real Alpha Juno synth via MIDI"

GST_DEBUG_CATEGORY (alphajunoctl_debug);

enum { MAX_VOICES = 6 };

typedef struct
{
  GstBtAudioSynthClass parent_class;
} GstBtAlphaJunoCtlClass;

// Class instance data.
typedef struct
{
  GstBtAudioSynth parent;

  int fd_midi;
  
  guint midi_device_no;
  guint benderRange;
  gboolean chorus;
  guint chorusRate;
  GstAlphaJunoCtlDcoRange dcoRange;
  guint dcoNoise;
  guint dcoAftertouch;
  guint dcoLFOMod;
  GstAlphaJunoCtlDcoEnvMode dcoEnvMode;
  guint dcoEnvMod;
  guint dcoPWMDepth;
  guint dcoPWMRate;
  guint dcoWavePulse;
  guint dcoWaveSaw;
  guint dcoWaveSub;
  guint dcoSubLevel;
  guint envT1AttackTime;
  guint envT1AttackLevel;
  guint envT2BreakTime;
  guint envT2BreakLevel;
  guint envT3DecayTime;
  guint envT3SustainLevel;
  guint envT4ReleaseTime;
  guint envKeyFollow;
  guint hpfCutoff;
  guint lfoRate;
  guint lfoDelay;
  guint vcfAftertouch;
  guint vcfCutoff;
  guint vcfResonance;
  GstAlphaJunoCtlVcfEnvMode vcfEnvMode;
  guint vcfKeyFollow;
  guint vcfLFOMod;
  guint vcfEnvMod;
  guint vcaAftertouch;
  GstAlphaJunoCtlVcaEnvMode vcaEnvMode;
  guint vcaLevel;
  guint pitchBend;
  gboolean portamento;
  guint portamentoTime;
  GstAlphaJunoCtlPolyMode polyMode;
  guint mainVolume;

  GstBtAlphaJunoCtlV *voices[MAX_VOICES];
  gulong cntVoices;
} GstBtAlphaJunoCtl;


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
  G_IMPLEMENT_INTERFACE (GSTBT_TYPE_CHILD_BIN, NULL))

static gboolean plugin_init(GstPlugin * plugin) {
  GST_DEBUG_CATEGORY_INIT(
	GST_CAT_DEFAULT,
	GST_MACHINE_NAME,
	GST_DEBUG_FG_WHITE | GST_DEBUG_BG_BLACK,
	GST_MACHINE_DESC);

  return gst_element_register(
	plugin,
	GST_MACHINE_NAME,
	GST_RANK_NONE,
	gstbt_alphajunoctl_get_type());
}

GST_PLUGIN_DEFINE(
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  alphajunoctl,
  GST_MACHINE_DESC,
  plugin_init, VERSION, "GPL", PACKAGE_NAME, PACKAGE_BUGREPORT)


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

inline void _midi_write_byte(int fd, int midichannel, unsigned char status, unsigned char control,
							 unsigned short value) {
  const guchar cmd[] = {(status << 4) | midichannel, control, value};
  _midi_write(fd, cmd, sizeof(cmd));
}

inline void _midi_write_switch(int fd, int midichannel, unsigned char status, unsigned char control,
							   unsigned short value) {
  const guchar cmd[] = {(status << 4) | midichannel, control, value << 6};
  _midi_write(fd, cmd, sizeof(cmd));
}

inline void _midi_write_14bit(int fd, int midichannel, unsigned char status, unsigned short value) {
  const guchar cmd[] = {(status << 4) | midichannel, value & 0x007F, value >> 7};
  _midi_write(fd, cmd, sizeof(cmd));
}

inline void _midi_write_toneparam(int fd, int midichannel, unsigned char param, unsigned char value) {
  const guchar cmd[] = {0xF0, 0x41, 0x36, midichannel, 0x23, 0x20, 0x01, param, value, 0xF7};
  _midi_write(fd, cmd, sizeof(cmd));
}


enum
{
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
  PROP_MIDI_DEVICE_NO,
  N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

static void _noteall_off(GstBtAlphaJunoCtl* self) {
  for (int i = 0; i < MAX_VOICES; ++i) {
	gstbt_alphajunoctlv_noteall_off(self->voices[i]);
  }
}

static void _retransmit(GstBtAlphaJunoCtl* const self) {
  _noteall_off(self);
  
  for (int i = PROP_BENDERRANGE; i <= PROP_MAINVOLUME; ++i) {
	const gchar* name = g_param_spec_get_name(properties[i]);
	GValue val = G_VALUE_INIT;
	g_object_get_property((GObject*)self, name, &val);
	g_object_set_property((GObject*)self, name, &val);
	g_value_unset(&val);
  }
}

static void _set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec) {
  GstBtAlphaJunoCtl *self = GSTBT_ALPHAJUNOCTL (object);

  switch (prop_id) {
  case PROP_CHILDREN:
	self->cntVoices = g_value_get_ulong(value);
	break;
  case PROP_MIDI_DEVICE_NO: {
	guint new_val = g_value_get_uint(value);
	if (self->fd_midi != -1 && new_val == self->midi_device_no)
	  break;
	
	self->midi_device_no = new_val;
	if (self->fd_midi != -1) {
	  close(self->fd_midi);
	  self->fd_midi = -1;
	}
	
	char device[12];
	if (self->midi_device_no > 0)
	  snprintf(device, sizeof(device), "/dev/midi%d", self->midi_device_no);
	else
	  strncpy(device, "/dev/midi", sizeof(device));
	  
	self->fd_midi = open(device, O_WRONLY);
	if (self->fd_midi == -1)
	  g_warning("MIDI device '%s' failed to open (%s)", device, strerror(errno));
	else
	  _retransmit(self);
  }
	break;
  case PROP_BENDERRANGE:
	self->benderRange = g_value_get_uint(value);
	_midi_write_toneparam(self->fd_midi, 0, 0x23, self->benderRange);
	break;
  case PROP_CHORUS:
    self->chorus = g_value_get_boolean(value);
	_midi_write_toneparam(self->fd_midi, 0, 0x0A, self->chorus);
    break;
  case PROP_CHORUSRATE:
    self->chorusRate = g_value_get_uint(value);
	_midi_write_toneparam(self->fd_midi, 0, 0x22, self->chorusRate);
    break;
  case PROP_DCORANGE:
    self->dcoRange = g_value_get_enum(value);
	_midi_write_toneparam(self->fd_midi, 0, 0x06, self->dcoRange);
    break;
  case PROP_DCONOISE:
    self->dcoNoise = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x08, self->dcoNoise);
    break;
  case PROP_DCOAFTERTOUCH:
    self->dcoAftertouch = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x0D, self->dcoAftertouch);
    break;
  case PROP_DCOLFOMOD:
    self->dcoLFOMod = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x0B, self->dcoLFOMod);
    break;
  case PROP_DCOENVMODE:
    self->dcoEnvMode = g_value_get_enum(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x00, self->dcoEnvMode);
    break;
  case PROP_DCOENVMOD:
    self->dcoEnvMod = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x0C, self->dcoEnvMod);
    break;
  case PROP_DCOPWMDEPTH:
    self->dcoPWMDepth = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x0E, self->dcoPWMDepth);
    break;
  case PROP_DCOPWMRATE:
    self->dcoPWMRate = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x0F, self->dcoPWMRate);
    break;
  case PROP_DCOWAVEPULSE:
    self->dcoWavePulse = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x03, self->dcoWavePulse);
    break;
  case PROP_DCOWAVESAW:
    self->dcoWaveSaw = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x04, self->dcoWaveSaw);
    break;
  case PROP_DCOWAVESUB:
    self->dcoWaveSub = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x05, self->dcoWaveSub);
    break;
  case PROP_DCOSUBLEVEL:
    self->dcoSubLevel = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x07, self->dcoSubLevel);
    break;
  case PROP_ENVT1ATTACKTIME:
    self->envT1AttackTime = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x1A, self->envT1AttackTime);
    break;
  case PROP_ENVT1ATTACKLEVEL:
    self->envT1AttackLevel = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x1B, self->envT1AttackLevel);
    break;
  case PROP_ENVT2BREAKTIME:
    self->envT2BreakTime = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x1C, self->envT2BreakTime);
    break;
  case PROP_ENVT2BREAKLEVEL:
    self->envT2BreakLevel = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x1D, self->envT2BreakLevel);
    break;
  case PROP_ENVT3DECAYTIME:
    self->envT3DecayTime = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x1E, self->envT3DecayTime);
    break;
  case PROP_ENVT3SUSTAINLEVEL:
    self->envT3SustainLevel = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x1F, self->envT3SustainLevel);
    break;
  case PROP_ENVT4RELEASETIME:
    self->envT4ReleaseTime = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x20, self->envT4ReleaseTime);
    break;
  case PROP_ENVKEYFOLLOW:
    self->envKeyFollow = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x21, self->envKeyFollow);
    break;
  case PROP_HPFCUTOFF:
    self->hpfCutoff = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x09, self->hpfCutoff);
    break;
  case PROP_LFORATE:
    self->lfoRate = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x18, self->lfoRate);
    break;
  case PROP_LFODELAY:
    self->lfoDelay = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x19, self->lfoDelay);
    break;
  case PROP_VCFAFTERTOUCH:
    self->vcfAftertouch = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x17, self->vcfAftertouch);
    break;
  case PROP_VCFCUTOFF:
    self->vcfCutoff = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x10, self->vcfCutoff);
    break;
  case PROP_VCFRESONANCE:
    self->vcfResonance = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x11, self->vcfResonance);
    break;
  case PROP_VCFENVMODE:
    self->vcfEnvMode = g_value_get_enum(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x01, self->vcfEnvMode);
    break;
  case PROP_VCFKEYFOLLOW:
    self->vcfKeyFollow = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x14, self->vcfKeyFollow);
    break;
  case PROP_VCFLFOMOD:
    self->vcfLFOMod = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x12, self->vcfLFOMod);
    break;
  case PROP_VCFENVMOD:
    self->vcfEnvMod = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x13, self->vcfEnvMod);
    break;
  case PROP_VCAAFTERTOUCH:
    self->vcaAftertouch = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x15, self->vcaAftertouch);
    break;
  case PROP_VCAENVMODE:
    self->vcaEnvMode = g_value_get_enum(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x02, self->vcaEnvMode);
    break;
  case PROP_VCALEVEL:
    self->vcaLevel = g_value_get_uint(value);
    _midi_write_toneparam(self->fd_midi, 0, 0x16, self->vcaLevel);
    break;
  case PROP_PITCHBEND:
    self->pitchBend = g_value_get_uint(value);
    _midi_write_14bit(self->fd_midi, 0, 0x0E, self->pitchBend);
    break;
  case PROP_PORTAMENTO:
    self->portamento = g_value_get_boolean(value);
    _midi_write_switch(self->fd_midi, 0, 0x0B, 0x41, self->portamento);
    break;
  case PROP_PORTAMENTOTIME:
    self->portamentoTime = g_value_get_uint(value);
    _midi_write_byte(self->fd_midi, 0, 0x0B, 0x05, self->portamentoTime);
    break;
  case PROP_POLYMODE:
    self->polyMode = g_value_get_enum(value);
    _midi_write_byte(self->fd_midi, 0, 0x0A, 0x01, self->polyMode);
    break;
  case PROP_MAINVOLUME:
    self->mainVolume = g_value_get_uint(value);
    _midi_write_byte(self->fd_midi, 0, 0x0B, 0x07, self->mainVolume);
    break;
  default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
  }
}

static void _get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec) {
  GstBtAlphaJunoCtl *self = GSTBT_ALPHAJUNOCTL (object);

  switch (prop_id) {
  case PROP_CHILDREN:
	g_value_set_ulong(value, self->cntVoices);
	break;
  case PROP_MIDI_DEVICE_NO:
	g_value_set_uint(value, self->midi_device_no);
	break;
  case PROP_BENDERRANGE:
	g_value_set_uint(value, self->benderRange);
	break;
  case PROP_CHORUS:
    g_value_set_boolean(value, self->chorus);
    break;
  case PROP_CHORUSRATE:
    g_value_set_uint(value, self->chorusRate);
    break;
  case PROP_DCORANGE:
    g_value_set_enum(value, self->dcoRange);
    break;
  case PROP_DCONOISE:
    g_value_set_uint(value, self->dcoNoise);
    break;
  case PROP_DCOAFTERTOUCH:
    g_value_set_uint(value, self->dcoAftertouch);
    break;
  case PROP_DCOLFOMOD:
    g_value_set_uint(value, self->dcoLFOMod);
    break;
  case PROP_DCOENVMODE:
    g_value_set_enum(value, self->dcoEnvMode);
    break;
  case PROP_DCOENVMOD:
    g_value_set_uint(value, self->dcoEnvMod);
    break;
  case PROP_DCOPWMDEPTH:
    g_value_set_uint(value, self->dcoPWMDepth);
    break;
  case PROP_DCOPWMRATE:
    g_value_set_uint(value, self->dcoPWMRate);
    break;
  case PROP_DCOWAVEPULSE:
    g_value_set_uint(value, self->dcoWavePulse);
    break;
  case PROP_DCOWAVESAW:
    g_value_set_uint(value, self->dcoWaveSaw);
    break;
  case PROP_DCOWAVESUB:
    g_value_set_uint(value, self->dcoWaveSub);
    break;
  case PROP_DCOSUBLEVEL:
    g_value_set_uint(value, self->dcoSubLevel);
    break;
  case PROP_ENVT1ATTACKTIME:
    g_value_set_uint(value, self->envT1AttackTime);
    break;
  case PROP_ENVT1ATTACKLEVEL:
    g_value_set_uint(value, self->envT1AttackLevel);
    break;
  case PROP_ENVT2BREAKTIME:
    g_value_set_uint(value, self->envT2BreakTime);
    break;
  case PROP_ENVT2BREAKLEVEL:
    g_value_set_uint(value, self->envT2BreakLevel);
    break;
  case PROP_ENVT3DECAYTIME:
    g_value_set_uint(value, self->envT3DecayTime);
    break;
  case PROP_ENVT3SUSTAINLEVEL:
    g_value_set_uint(value, self->envT3SustainLevel);
    break;
  case PROP_ENVT4RELEASETIME:
    g_value_set_uint(value, self->envT4ReleaseTime);
    break;
  case PROP_ENVKEYFOLLOW:
    g_value_set_uint(value, self->envKeyFollow);
    break;
  case PROP_HPFCUTOFF:
    g_value_set_uint(value, self->hpfCutoff);
    break;
  case PROP_LFORATE:
    g_value_set_uint(value, self->lfoRate);
    break;
  case PROP_LFODELAY:
    g_value_set_uint(value, self->lfoDelay);
    break;
  case PROP_VCFAFTERTOUCH:
    g_value_set_uint(value, self->vcfAftertouch);
    break;
  case PROP_VCFCUTOFF:
    g_value_set_uint(value, self->vcfCutoff);
    break;
  case PROP_VCFRESONANCE:
    g_value_set_uint(value, self->vcfResonance);
    break;
  case PROP_VCFENVMODE:
    g_value_set_enum(value, self->vcfEnvMode);
    break;
  case PROP_VCFKEYFOLLOW:
    g_value_set_uint(value, self->vcfKeyFollow);
    break;
  case PROP_VCFLFOMOD:
    g_value_set_uint(value, self->vcfLFOMod);
    break;
  case PROP_VCFENVMOD:
    g_value_set_uint(value, self->vcfEnvMod);
    break;
  case PROP_VCAAFTERTOUCH:
    g_value_set_uint(value, self->vcaAftertouch);
    break;
  case PROP_VCAENVMODE:
    g_value_set_enum(value, self->vcaEnvMode);
    break;
  case PROP_VCALEVEL:
    g_value_set_uint(value, self->vcaLevel);
    break;
  case PROP_PITCHBEND:
    g_value_set_uint(value, self->pitchBend);
    break;
  case PROP_PORTAMENTO:
    g_value_set_boolean(value, self->portamento);
    break;
  case PROP_PORTAMENTOTIME:
    g_value_set_uint(value, self->portamentoTime);
    break;
  case PROP_POLYMODE:
    g_value_set_enum(value, self->polyMode);
    break;
  case PROP_MAINVOLUME:
    g_value_set_uint(value, self->mainVolume);
    break;
  default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
  }
}

static gboolean _process (GstBtAudioSynth* synth, GstBuffer* gstbuf, GstMapInfo* info) {
  GstBtAlphaJunoCtl *self = GSTBT_ALPHAJUNOCTL(synth);

  // Parameter group's pattern control source seems not to be called.
  for (int i = 0; i < self->cntVoices; ++i) {
	gstbt_alphajunoctlv_process(self->voices[i], gstbuf);
    gst_object_sync_values((GstObject*)self->voices[i], GST_BUFFER_TIMESTAMP(gstbuf));
  }

  memset(info->data, 0, synth->generate_samples_per_buffer*sizeof(gint16));
  return TRUE;
}

static void _negotiate (GstBtAudioSynth* base, GstCaps* caps) {
  for (gint i = 0; i < gst_caps_get_size(caps); ++i) {
    gst_structure_fixate_field_nearest_int(
	  gst_caps_get_structure (caps, i),
	  "channels",
	  1);
  }
}

static void _dispose (GObject* object) {
  GstBtAlphaJunoCtl* self = GSTBT_ALPHAJUNOCTL(object);
  
  // It's necessary to unparent children so they will be unreffed and cleaned up. GstObject doesn't hold variable
  // links to its children, so wouldn't know to unparent them.
  for (int i = 0; i < MAX_VOICES; i++) {
	gst_object_unparent((GstObject*)self->voices[i]);
  }

  G_OBJECT_CLASS(gstbt_alphajunoctl_parent_class)->dispose(object);
}

static void gstbt_alphajunoctl_class_init(GstBtAlphaJunoCtlClass * const klass) {
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
	PACKAGE_BUGREPORT);

  GstBtAudioSynthClass *audio_synth_class = (GstBtAudioSynthClass *) klass;
  audio_synth_class->process = _process;
  /*audio_synth_class->reset = gstbt_sim_syn_reset;*/
  audio_synth_class->negotiate = _negotiate;

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
	g_param_spec_boolean("chorus", "Chorus", "", FALSE, flags);
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

  properties[PROP_MIDI_DEVICE_NO] = 
	g_param_spec_uint("midi-device-no", "MIDI Device", "", 0, 20, 0, flags);
  
  for (int i = 1; i < N_PROPERTIES; ++i)
	g_assert(properties[i]);
  
  g_object_class_install_properties (gobject_class, N_PROPERTIES, properties);
}

static void gstbt_alphajunoctl_init (GstBtAlphaJunoCtl* const self) {
  self->fd_midi = -1;
  
  for (int i = 0; i < MAX_VOICES; i++) {
	self->voices[i] = gstbt_alphajunoctlv_new(i);

	char name[7];
	snprintf(name, sizeof(name), "voice%1d", i);
		
	gst_object_set_name((GstObject *)self->voices[i], name);
	gst_object_set_parent((GstObject *)self->voices[i], (GstObject *)self);
  }
}

int gstbt_alphajunoctl_midiout_get(GstBtAlphaJunoCtl* const self) {
  return self->fd_midi;
}
