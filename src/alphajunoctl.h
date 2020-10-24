#pragma once

#include "src/genums.h"
#include "src/voice.h"
#include "libbuzztrax-gst/audiosynth.h"
#include "libbuzztrax-gst/toneconversion.h"
#include <gst/gst.h>

G_BEGIN_DECLS
#define GSTBT_ALPHAJUNOCTL(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),gstbt_alphajunoctl_get_type(),GstBtAlphaJunoCtl))

#define MAX_VOICES 6

// Class instance data.
typedef struct
{
	GstBtAudioSynth parent;
	
	//  GstBtToneConversionTuning tuning;
	//  GstBtToneConversion *n2f;

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

typedef struct 
{
  GstBtAudioSynthClass parent_class;
} GstBtAlphaJunoCtlClass;

GType gstbt_alphajunoctl_get_type (void);

G_END_DECLS
