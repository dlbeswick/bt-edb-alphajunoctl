#pragma once
// These enums will have GObject code generated for them by the `glib-mkenums' tool.

typedef enum
{
	DCORANGE_4 = 0, /*< nick='4 >*/
	DCORANGE_8,     /*< nick='8 >*/
	DCORANGE_16,    /*< nick='16 >*/
	DCORANGE_32     /*< nick='32 >*/
} GstAlphaJunoCtlDcoRange;

typedef enum
{
	DCOENVMODE_NORMAL = 0, /*< nick=Normal >*/
	DCOENVMODE_INVERTED, /*< nick=Inverted >*/
	DCOENVMODE_NORMALDYNAMIC, /*< nick=Normal-Dynamic >*/
	DCOENVMODE_INVDYNAMIC /*< nick=Inv-Dynamic >*/
} GstAlphaJunoCtlDcoEnvMode;

typedef enum
{
	VCFENVMODE_NORMAL = 0, /*< nick=Normal >*/
	VCFENVMODE_INVERTED, /*< nick=Inverted >*/
	VCFENVMODE_NORMALDYNAMIC, /*< nick=Normal-Dynamic >*/
	VCFENVMODE_DYNAMIC, /*< nick=Dynamic >*/
} GstAlphaJunoCtlVcfEnvMode;

typedef enum
{
	VCAENVMODE_NORMAL = 0, /*< nick=Normal >*/
	VCAENVMODE_GATE, /*< nick=Gate >*/
	VCAENVMODE_NORMALDYNAMIC, /*< nick=Normal-Dynamic >*/
	VCAENVMODE_GATEDYNAMIC, /*< nick=Gate-Dynamic >*/
} GstAlphaJunoCtlVcaEnvMode;


typedef enum
{
	POLYMODE_OMNIOFF = 0, /*< nick=Omni Off >*/
	POLYMODE_OMNION, /*< nick=Omni On >*/
	POLYMODE_MONOON, /*< nick=Mono On >*/
	POLYMODE_POLYON, /*< nick=Poly On >*/
} GstAlphaJunoCtlPolyMode;
