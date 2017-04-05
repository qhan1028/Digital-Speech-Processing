#ifndef __TEST_HMM__
#define __TEST_HMM__

#include "hmm.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*	Simulator of HMM in Speech Recognition
 *	
 *	Original Variables:
 *	d[t][i]		The highest probability along a certain single path ending 
 *				at state i at time t for the first t observations, given 
 *				Pi, A, B.
 */

/* max number of all states */
static int nr_state;
static int nr_time;

/* use a better variable name */
static double *Pi;
static double (*A)[MAX_STATE];
static double (*B)[MAX_STATE];

/* testing data */
static char test_seq[MAX_LINE];

/* declaration of delta */
static double d[MAX_LINE][MAX_STATE];

static int obsv_to_num (char c) { return c - 'A';}

static int max_d(int t, int j)
{
	double max = 0;
	int max_idx = 0;
	for (int i = 0 ; i < nr_state ; i++) {
		if (d[t][i] * A[i][j] > max) {
			max = d[t][i] * A[i][j];
			max_idx = i;
		}
	}
	return max_idx;
}

static void delta()
{
	/* the observed o at time t0 */
	int o = obsv_to_num(test_seq[0]);
	for (int i = 0 ; i < nr_state ; i++) {
		d[0][i] = Pi[i] * B[o][i];
	}

	/* the observed o at time t1 to T */
	for (int t = 1 ; t < nr_time ; t++) {
		o = obsv_to_num(test_seq[t]);

		for (int j = 0 ; j < nr_state ; j++) {
			int i = max_d(t-1, j);
			d[t][j] = d[t-1][i] * A[i][j] * B[o][j];
		}
	}
}

static double test_core(HMM *hmm)
{
	nr_state = hmm->state_num;
	nr_time = strlen(test_seq);

	/* assign hmm data structure to new variable name */
	Pi = (hmm->initial);
	A = (hmm->transition);
	B = (hmm->observation);

	memset(d, 0, sizeof(d));
	
	/* start calculation */
	delta();

	/* find max probability of d[T][i] */
	int T = nr_time - 1;
	double max_d = 0;
	for (int i = 0 ; i < nr_state ; i++) {
		if (d[T][i] > max_d) {
			max_d = d[T][i];
		}
	}
	return max_d;
}

static void test(HMM hmms[], FILE * test_fp, FILE * result_fp, int nr_model)
{
	while (fscanf(test_fp, "%s", test_seq) != EOF) {
		int result_model = 0;
		double max_prob = 0;
		double tmp_prob = 0;
			
		/* iterate for each model */
		for (int k = 0 ; k < nr_model ; k++) {
			tmp_prob = test_core(&hmms[k]);

			/* update the most possible model */
			if (tmp_prob > max_prob) {
				max_prob = tmp_prob;
				result_model = k;
			}
		}
		/* output to result */
		fprintf(result_fp, "%s %.6e\n", hmms[result_model].model_name, max_prob);
	}
}

int main(int argc, char *argv[])
{
	/* parse parameters */
	char *list_file = argv[1];
	char *test_data = argv[2];
	char *result_file = argv[3];

	/* load models */
	HMM hmms[5];
	load_models(list_file , hmms, 5);

	/* open test data */
	FILE *test_fp = open_or_die(test_data, "r");

	/* open result file */
	FILE *result_fp = open_or_die(result_file, "w");

	test(hmms, test_fp, result_fp, 5);

	return 0;
}

#endif
