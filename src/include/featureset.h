#ifndef _FEATURESET_H
#define _FEATURESET_H

#include <fftw3.h>
#include <math.h>

#include "svm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DATA_RANGE 256
#define EPSILON 0.00000000000000000001
#define LOGOF2	0.69314718055994530941

/* feature set used by the classifier */
typedef struct _featureset {
	double *histogram;
	double *fftin;
	unsigned int fftout_size;
	fftw_complex *fftout;
	fftw_plan plan;

	/* features */
	double mean;
	double variance;
	double entropy;
	double power;		/* power of 1/8 band */
	double fmean;		/* mean of freq domain */
	double fvariance;
} featureset;

struct svmscale {
	double min;
	double max;
};

static struct svmscale fscale[] = {
	{0.000007, 0.015558},
	{0.000007, 0.946698},
	{0.0, 0.000488},
	{0.115077, 113845000.0},
	{0.001154, 0.695863},
	{0.005887, 28702.6}
};

void init_featureset(featureset *, unsigned int);
void finit_featureset(featureset *);
void compute_features(const unsigned char *, unsigned int, featureset *);

static inline void normalize_features(featureset * f, unsigned int n)
{
	f->mean = f->mean / n;
	f->variance = f->variance / n;
	f->entropy = f->entropy / n;
	f->power = f->power / n;
	f->fmean = f->fmean / n;
	f->fvariance = f->fvariance / n;
}

static inline void init_svmnode(const featureset * f, struct svm_node *x)
{

	x[0].index = 1;
	x[0].value =
	    (-1) +
	    ((2) * (f->mean - fscale[0].min) / (fscale[0].max - fscale[0].min));
	x[1].index = 2;
	x[1].value =
	    (-1) +
	    ((2) * (f->variance - fscale[1].min) /
	     (fscale[1].max - fscale[1].min));
	x[2].index = 3;
	x[2].value =
	    (-1) +
	    ((2) * (f->entropy - fscale[2].min) /
	     (fscale[2].max - fscale[2].min));
	x[3].index = 4;
	x[3].value =
	    (-1) +
	    ((2) * (f->power - fscale[3].min) /
	     (fscale[3].max - fscale[3].min));
	x[4].index = 5;
	x[4].value =
	    (-1) +
	    ((2) * (f->fmean - fscale[4].min) /
	     (fscale[4].max - fscale[4].min));
	x[5].index = 6;
	x[5].value =
	    (-1) +
	    ((2) * (f->fvariance - fscale[5].min) /
	     (fscale[5].max - fscale[5].min));
	x[6].index = -1;
	x[6].value = 0.0;
}

/* // this causes warnings, and is never used...
static inline char *svmtohuman(int v)
{

	switch (v) {
	case 1:
		return "Plain-Text";
		break;

	case 2:
		return "Image-BMP";
		break;

	case 3:
		return "Audio-WAV";
		break;

	case 4:
		return "Compressed";
		break;

	case 5:
		return "Image-JPG";
		break;

	case 6:
		return "Audio-MP3";
		break;

	case 7:
		return "Video-MPG";
		break;

	case 8:
		return "Encrypted";
		break;

	default:
		return "Unknown (SVM Confused!)";
		break;
	}
}
*/

#ifdef __cplusplus
}
#endif

#endif
