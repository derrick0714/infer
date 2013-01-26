#include <stdlib.h>
#include <string.h>
#include "featureset.h"

void init_featureset(featureset * f, unsigned int size)
{

	memset(f, 0, sizeof(featureset));

	if ((f->fftin = (double *)malloc((size * sizeof(double)))) == NULL) {
		fprintf(stderr, "init_featureset: could not allocate memory\n");
		return;
	}

	if ((f->histogram =
	     (double *)malloc((DATA_RANGE * sizeof(double)))) == NULL) {
		fprintf(stderr, "init_featureset: could not allocate memory\n");
		return;
	}

	f->fftout_size = ((size / 2) + 1);
	f->fftout = (fftw_complex*)fftw_malloc(f->fftout_size * sizeof(fftw_complex));
	f->plan = fftw_plan_dft_r2c_1d(size, f->fftin, f->fftout, FFTW_MEASURE);
}

void finit_featureset(featureset * f)
{
	free(f->fftin);
	free(f->histogram);
	fftw_free(f->fftout);
	fftw_destroy_plan(f->plan);
	fftw_cleanup();
}

void compute_features(const unsigned char *data, unsigned int size,
		      featureset * f)
{
	register int i;
	register unsigned char *c;
	register double *fftin, *histo;

	unsigned int band, fftout_size;
	double sum, tmp, tmp1, var;
	fftw_complex *fftout;

	fftout_size = f->fftout_size;
	fftin = f->fftin;
	fftout = f->fftout;
	histo = f->histogram;
	memset(histo, 0, DATA_RANGE * sizeof(double));

	/* compute mean, histogram, casting */
	sum = 0.0;
	c = (unsigned char *)data;
	for (i = (size - 1); (i >= 0); --i, ++c, ++fftin) {
		*fftin = (double)*c;
		sum += *fftin;
		++(histo[*c]);
	}
	f->mean = (sum / (double)size);

	/* compute variance */
	sum = f->mean;
	fftin = f->fftin;
	for (i = 0, tmp1 = 0.0, var = 0.0; (i < size); ++i, ++fftin) {
		tmp = (*fftin - sum);
		tmp1 += tmp;
		var += (tmp * tmp);
	}
	f->variance = ((var - ((tmp1 * tmp1) / size)) / (size - 1));

	/* compute entropy */
	for (i = 0, tmp = 0.0; (i < DATA_RANGE); ++i, ++histo) {
		*histo = (*histo / (double)size);
		*histo = (*histo + EPSILON);
		tmp -= (*histo * (log(*histo) / LOGOF2));
	}
	f->entropy = tmp;

	/* FFT */
	fftw_execute(f->plan);

	/* get abs() of fftout and reuse fftin */
	fftin = f->fftin;
	for (i = 0; (i < fftout_size); ++i, ++fftin)
		*fftin =
		    sqrt(((fftout[i][0] * fftout[i][0]) +
			  (fftout[i][1] * fftout[i][1])));

	/* compute power of the (1/8)*fftout_size */
	band = (fftout_size >> 3);
	fftin = f->fftin;
	for (i = 1, tmp = 0.0; (i <= band); ++i)
		tmp += ((fftin[i]) * (fftin[i]));

	f->power = tmp;

	/* compute fmean and fvaraiance on the second half of fftout */
	band = (fftout_size >> 1);
	fftin = f->fftin;
	for (i = (band + 1), sum = 0.0; (i < fftout_size); ++i)
		sum += fftin[i];
	f->fmean = (sum / (double)(fftout_size - band));

	/* compute fvariance */
	tmp1 = f->fmean;
	for (i = (band + 1), sum = 0.0, var = 0.0; (i < fftout_size); ++i) {
		tmp = (fftin[i] - tmp1);
		sum += tmp;
		var += (tmp * tmp);
	}
	f->fvariance =
	    ((var -
	      ((sum * sum) / (fftout_size >> 1))) / ((fftout_size - 1) >> 1));
}
