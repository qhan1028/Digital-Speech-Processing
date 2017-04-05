#ifndef __TRAIN_HMM__
#define __TRAIN_HMM__

#include "hmm.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/*	Simulator of HMM in Speech Recognition 
 *	
 *	Original Variables:
 *	Pi[i]		The vector of initial state [i] probabilities.
 *	A[i][j]		The N * N matrix of state transition from [i] to [j] 
 *				probabilities.
 *	B[o][i]		A set of N probability functions, each describing the 
 * 				observation [o] probability with respect to a state [i].
 *	a[t][i]		Forward variable, probability of observing o(0) ~ o(t) 
 * 				with state [i] at time [t], while using Pi, A, B.
 *	b[t][i]		Backward variable, probability of observing o(t+1) ~ 
 *				o(T), while state [i] at time [t] and using Pi, A, B.
 *	r[t][i]		Combination of a[t][i] and b[t][i], probability of state
 *				[i] at time [t], while observing o(0) ~ o(T) and using
 *				Pi, A, B.
 *	e[t][i][j]	Probability of state [i] at time [t] and state [j] at
 *				time [t+1], while observing o(0) ~ o(T) and using Pi,
 *				A, B.
 */


/* max number of all states */
static int nr_state;
static int nr_obsv;
static int nr_time;

/* use a better variable name */
static double *Pi;
static double (*A)[MAX_STATE];
static double (*B)[MAX_STATE];

/* training data */
static char train_seq[MAX_LINE];

/* declaration of alpha, beta, gamma, epsilon */
static double a[MAX_LINE][MAX_STATE];			/* alpha */
static double b[MAX_LINE][MAX_STATE];			/* beta */

static double sum_r[MAX_STATE];				/* sum of all gamma with state i */
static double sum_ro[MAX_OBSERV][MAX_STATE];	/* sum of all gamma with state i & observed o */
static double sum_e[MAX_STATE][MAX_STATE];		/* sum of all epsilon with state i to j */

static double new_Pi[MAX_STATE];				/* new Pi */
static int count;								/* number of samples */

static int obsv_to_num (char c) { return c - 'A';}

static void alpha()
{
	/* initial state at t0 */
	int o = obsv_to_num(train_seq[0]);
	for (int i = 0 ; i < nr_state ; i++) {
		a[0][i] = Pi[i] * B[o][i];
	}

	/* start calculate a */
	for (int t = 1 ; t < nr_time ; t++) {
		/* the current observed alphabet */
		o = obsv_to_num(train_seq[t]);

		for (int j = 0 ; j < nr_state ; j++) {
			/* sigma i ( a[t-1][i] * A[i][j] ) */
			double sigma = 0;
			for (int i = 0 ; i < nr_state ; i++)
				sigma += a[t-1][i] * A[i][j];

			a[t][j] = sigma * B[o][j];
		}
	}
}

static void beta()
{
	/* init beta at t = T */
	int T = nr_time - 1;
	for (int i = 0 ; i < nr_state ; i++) 
		b[T][i] = 1;

	/* start calculate b */
	int o;
	for (int t = T-1 ; t >= 0 ; t--) {
		/* the next observed alphabet */
		o = obsv_to_num(train_seq[t+1]);

		for (int i = 0 ; i < nr_state ; i++) {
			/* sigma j ( A[i][j] * B[o][j] * b[t+1][j]) */
			double sigma = 0;
			for (int j = 0 ; j < nr_state ; j++)
				sigma += A[i][j] * B[o][j] * b[t+1][j];

			b[t][i] = sigma;
		}
	}
}

static void gama()
{
	/* start calculate one sample r, and add them to sum_r & sum_ro */
	double tmp_r[nr_state];
	int o;

	for (int t = 0 ; t < nr_time ; t++) {
		/* the observed o at time t */
		o = obsv_to_num(train_seq[t]);

		double tmp_sum = 0;
		for (int i = 0 ; i < nr_state ; i++) {
			tmp_r[i] = a[t][i] * b[t][i];			/* numerator of r of single sample */
			tmp_sum += tmp_r[i];					/* dominator of r of single sample */
		}
		for (int i = 0 ; i < nr_state ; i++) {
			tmp_r[i] /= tmp_sum;					/* r of single sample */
			sum_r[i] += tmp_r[i];					/* sum of all r with state i */
			sum_ro[o][i] += tmp_r[i];				/* sum of all r with state i and observd o */
		}

		/* Pi */
		if (t == 0) {
			for (int i = 0 ; i < nr_state ; i++) {
				new_Pi[i] += tmp_r[i];
			}
		}
	}
}

static void epsilon()
{
	double tmp_e[nr_state][nr_state];
	int o;

	for (int t = 0 ; t < nr_time ; t++) {
		/* the observed o at time t */
		o = obsv_to_num(train_seq[t+1]);

		double tmp_sum = 0;
		for (int i = 0 ; i < nr_state ; i++) {
			for (int j = 0 ; j < nr_state ; j++) {
				tmp_e[i][j] = a[t][i] * A[i][j] * B[o][j] * b[t+1][j];	/* numerator of e of single sample */
				tmp_sum += tmp_e[i][j];									/* dominator of e of single sample */
			}
		}
		for (int i = 0 ; i < nr_state ; i++) {
			for (int j = 0 ; j < nr_state ; j++) {
				if (tmp_sum != 0.0) {
					tmp_e[i][j] /= tmp_sum;				/* e of single sample */
				}
				sum_e[i][j] += tmp_e[i][j];				/* sum of all e of state i to j */
			}
		}
	}
}

static void train_core()
{
	/* update Pi */
	for (int i = 0 ; i < nr_state ; i++) 
		Pi[i] = new_Pi[i] / count;

	/* update A */
	for (int i = 0 ; i < nr_state ; i++) {
		for (int j = 0 ; j < nr_state ; j++) {
			if (sum_r[i] != 0) {
				A[i][j] = sum_e[i][j] / sum_r[i];
			}
		}
	}

	/* update B */
	for (int o = 0 ; o < nr_obsv ; o++) {
		for (int i = 0 ; i < nr_state ; i++) {
			B[o][i] = sum_ro[o][i] / sum_r[i];
		}
	}
}

static void train_reset(FILE * train_fp)
{
	memset(new_Pi, 0.0, sizeof(double) * MAX_STATE);
	memset(sum_r, 0.0, sizeof(double) * MAX_STATE);
	memset(sum_e, 0.0, sizeof(double) * MAX_STATE * MAX_STATE);
	memset(sum_ro, 0.0, sizeof(double) * MAX_OBSERV * MAX_STATE);
	count = 0;
	fseek(train_fp, 0, SEEK_SET);
}

static void train(HMM *hmm, FILE *train_fp, int iteration)
{
	nr_state = hmm->state_num;
	nr_obsv = hmm->observ_num;
	nr_time = 0;

	/* assign hmm data structure to new variable name */
	Pi = (hmm->initial);
	A = (hmm->transition);
	B = (hmm->observation);

	/* init to 0 */
	train_reset(train_fp);

	for (int i = 0 ; i < iteration ; i++) {

		count = 0;	/* count for samples */
		while (fscanf(train_fp, "%s", train_seq) != EOF) {
			memset(a, 0.0, sizeof(a));
			memset(b, 0.0, sizeof(b));

			/* find the sequence length */
			if (count == 0)
				nr_time = strlen(train_seq);

			/* calculate alpha, beta per sample */
			alpha();
			beta();

			/* summerize r, e of all samples */
			gama();
			epsilon();

			count++;
		}
		train_core();
		train_reset(train_fp);
	}
}

int main(int argc, char *argv[])
{
	/* parse parameters */
	int iteration = atoi(argv[1]);
	char *init_file = argv[2];
	char *train_data = argv[3];
	char *result_file = argv[4];

	/* load initial HMM */
	HMM hmm;
	loadHMM(&hmm, init_file);

	/* open training data */
	FILE *fp = open_or_die(train_data, "r");

	/* train */
	train(&hmm, fp, iteration);

	/* output result */
	strcpy(hmm.model_name, result_file);
	dump_models(&hmm, 1);

	fclose(fp);
	
	return 0;
}

#endif
