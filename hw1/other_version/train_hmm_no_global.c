#include "hmm.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//#define DEBUG

int obsv_to_num (char c)
{
	return c - 'A';
}

void alpha(double *Pi, double A[][MAX_STATE], double B[][MAX_STATE], double a[][MAX_STATE], char *seq, int nr_state, int nr_time)
{
	/* initial state at t0 */
	int o = obsv_to_num(seq[0]);
	for (int i = 0 ; i < nr_state ; i++) {
		a[0][i] = Pi[i] * B[o][i];
	}

	/* start calculate a */
	for (int t = 1 ; t < nr_time ; t++) {
		/* the current observed alphabet */
		o = obsv_to_num(seq[t]);

		for (int j = 0 ; j < nr_state ; j++) {
			/* sigma i ( a[t-1][i] * A[i][j] ) */
			double sigma = 0;
			for (int i = 0 ; i < nr_state ; i++)
				sigma += a[t-1][i] * A[i][j];

			a[t][j] = sigma * B[o][j];
		}
	}
}

void beta(double A[][MAX_STATE], double B[][MAX_STATE], double b[][MAX_STATE], char *seq, int nr_state, int nr_time)
{
	/* init beta at t = T */
	int T = nr_time - 1;
	for (int i = 0 ; i < nr_state ; i++) 
		b[T][i] = 1;

	/* start calculate b */
	int o;
	for (int t = T-1 ; t >= 0 ; t--) {
		/* the next observed alphabet */
		o = obsv_to_num(seq[t+1]);

		for (int i = 0 ; i < nr_state ; i++) {
			/* sigma j ( A[i][j] * B[o][j] * b[t+1][j]) */
			double sigma = 0;
			for (int j = 0 ; j < nr_state ; j++)
				sigma += A[i][j] * B[o][j] * b[t+1][j];

			b[t][i] = sigma;
		}
	}
}

void gama(double a[][MAX_STATE], double b[][MAX_STATE], double sum_r[], double ro[][MAX_STATE], 
			double new_Pi[], char seq[], int nr_state, int nr_time)
{
	/* start calculate one sample r, and add them to sum_r & ro */
	double tmp_r[nr_state];
	int o;

	for (int t = 0 ; t < nr_time ; t++) {
		/* the observed o at time t */
		o = obsv_to_num(seq[t]);

		double tmp_sum = 0;
		for (int i = 0 ; i < nr_state ; i++) {
			tmp_r[i] = a[t][i] * b[t][i];			/* numerator of r of single sample */
			tmp_sum += tmp_r[i];					/* dominator of r of single sample */
		}
		for (int i = 0 ; i < nr_state ; i++) {
			tmp_r[i] /= tmp_sum;					/* r of single sample */
			sum_r[i] += tmp_r[i];					/* sum of all r with state i */
			ro[o][i] += tmp_r[i];					/* sum of all r with state i and observd o */
		}

		/* Pi */
		if (t == 0) {
			for (int i = 0 ; i < nr_state ; i++) {
				new_Pi[i] += tmp_r[i];
			}
		}
	}
}

void epsilon(double A[][MAX_STATE], double B[][MAX_STATE], double a[][MAX_STATE], double b[][MAX_STATE], 
			double sum_e[][MAX_STATE], char seq[], int nr_state, int nr_time)
{
	double tmp_e[nr_state][nr_state];
	int o;

	for (int t = 0 ; t < nr_time ; t++) {
		/* the observed o at time t */
		o = obsv_to_num(seq[t+1]);

		double tmp_sum = 0;
		for (int i = 0 ; i < nr_state ; i++) {
			for (int j = 0 ; j < nr_state ; j++) {
				tmp_e[i][j] = a[t][i] * A[i][j] * B[o][j] * b[t+1][j];	/* numerator of e of single sample */
				tmp_sum += tmp_e[i][j];									/* dominator of e of single sample */
			}
		}
		for (int i = 0 ; i < nr_state ; i++) {
			for (int j = 0 ; j < nr_state ; j++) {
				/* check if all e[i][j] is zero at time t */
				if (tmp_sum != 0)
					tmp_e[i][j] /= tmp_sum;				/* e of single sample */

				sum_e[i][j] += tmp_e[i][j];				/* sum of all e of state i to j */
			}
		}
	}
}

void train_core(double Pi[], double new_Pi[], double A[][MAX_STATE], double B[][MAX_STATE], double sum_r[], 
			double sum_e[][MAX_STATE], double sum_ro[][MAX_STATE], int nr_state, int nr_time, int nr_obsv, int nr_samples)
{
	/* update Pi */
	for (int i = 0 ; i < nr_state ; i++) 
		Pi[i] = new_Pi[i] / nr_samples;

	/* update A */
	for (int i = 0 ; i < nr_state ; i++) {
		for (int j = 0 ; j < nr_state ; j++) {
			A[i][j] = sum_e[i][j] / sum_r[i];
		}
	}

	/* update B */
	for (int o = 0 ; o < nr_obsv ; o++) {
		for (int i = 0 ; i < nr_state ; i++) {
			B[o][i] = sum_ro[o][i] / sum_r[i];
		}
	}
}

void train_reset(double new_Pi[], double sum_r[], double sum_e[][MAX_STATE], double sum_ro[][MAX_STATE], 
			int* count, FILE* train_fp)
{
	memset(new_Pi, 0, sizeof(double) * MAX_STATE);
	memset(sum_r, 0, sizeof(double) * MAX_STATE);
	memset(sum_e, 0, sizeof(double) * MAX_STATE * MAX_STATE);
	memset(sum_ro, 0, sizeof(double) * MAX_OBSERV * MAX_STATE);
	*count = 0;
	fseek(train_fp, 0, SEEK_SET);
}


void train(HMM *hmm, FILE *train_fp, int iteration)
{
	/* max number of all states */
	int nr_state = hmm->state_num;
	int nr_obsv = hmm->observ_num;
	int nr_time = 0;

	/* use a better variable name */
	double Pi[MAX_STATE];
	double A[MAX_STATE][MAX_STATE];
	double B[MAX_OBSERV][MAX_STATE];
	memcpy(Pi, &(hmm->initial), sizeof(hmm->initial));
	memcpy(A, &(hmm->transition), sizeof(hmm->transition));
	memcpy(B, &(hmm->observation), sizeof(hmm->observation));

#ifdef DEBUG
	fprintf(stderr, "\nBefore Training:\nPi:\n");
	for (int i = 0 ; i < hmm->state_num ; i++)
		fprintf(stderr, "%g ", Pi[i]);
	fprintf(stderr, "\n\nA:\n");

	for (int i = 0 ; i < hmm->state_num ; i++) {
		for (int j = 0 ; j < hmm->state_num ; j++)
			fprintf(stderr, "%g ", A[i][j]);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\nB:\n");
		
	for (int i = 0 ; i < hmm->observ_num ; i++) {
		for (int j = 0 ; j < hmm->state_num ; j++)
			fprintf(stderr, "%g ", B[i][j]);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
#endif

	/* training data */
	char train_seq[MAX_LINE];

	/* declaration of alpha, beta, gamma, epsilon */
	double a[MAX_LINE][MAX_STATE];	/* alpha */
	double b[MAX_LINE][MAX_STATE];	/* beta */

	double sum_r[MAX_STATE] = {0};	/* sum of all gamma with state i */
	double ro[MAX_OBSERV][MAX_STATE];	/* sum of all gamma with state i & observed o */
	memset(ro, 0, sizeof(ro));

	double sum_e[MAX_STATE][MAX_STATE];
	memset(sum_e, 0, sizeof(sum_e));

	double new_Pi[MAX_STATE];

	for (int i = 0 ; i < iteration ; i++) {
		/* one iteration */
		int count = 0;	/* count for samples */
		while (fscanf(train_fp, "%s", train_seq) != EOF) {
			memset(a, 0, sizeof(a));
			memset(b, 0, sizeof(b));
			/* find the sequence length */
			if (count == 0)
				nr_time = strlen(train_seq);

			/* calculate alpha, beta per sample */
			alpha(Pi, A, B, a, train_seq, nr_state, nr_time);
			beta(A, B, b, train_seq, nr_state, nr_time);

			/* summerize r, e of all samples */
			gama(a, b, sum_r, ro, new_Pi, train_seq, nr_state, nr_time);
			epsilon(A, B, a, b, sum_e, train_seq, nr_state, nr_time);

			count++;
		}
		train_core(Pi, new_Pi, A, B, sum_r, sum_e, ro, nr_state, nr_time, nr_obsv, count);
		train_reset(new_Pi, sum_r, sum_e, ro, &count, train_fp);
#ifdef DEBUG
	fprintf(stderr, "\nAfter Training #%d:\nPi:\n", i);
	for (int i = 0 ; i < hmm->state_num ; i++)
		fprintf(stderr, "%f ", Pi[i]);
	fprintf(stderr, "\n\nA:\n");

	for (int i = 0 ; i < hmm->state_num ; i++) {
		for (int j = 0 ; j < hmm->state_num ; j++)
			fprintf(stderr, "%f ", A[i][j]);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\nB:\n");
		
	for (int i = 0 ; i < hmm->observ_num ; i++) {
		for (int j = 0 ; j < hmm->state_num ; j++)
			fprintf(stderr, "%f ", B[i][j]);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
#endif
	}
	/* store back to hmm */
	memcpy(&(hmm->initial), Pi, sizeof(Pi));
	memcpy(&(hmm->transition), A, sizeof(A));
	memcpy(&(hmm->observation), B, sizeof(B));
	fprintf(stderr, "sucess\n");
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
	
	return 0;
}
