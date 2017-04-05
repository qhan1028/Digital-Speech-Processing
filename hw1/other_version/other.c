
#ifdef DEBUG
	for (int i = 0 ; i < hmm->state_num ; i++)
		fprintf(stderr, "%f ", Pi[i]);
	fprintf(stderr, "\n\n");

	for (int i = 0 ; i < hmm->state_num ; i++) {
		for (int j = 0 ; j < hmm->state_num ; j++)
			fprintf(stderr, "%f ", A[i][j]);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
		
	for (int i = 0 ; i < hmm->observ_num ; i++) {
		for (int j = 0 ; j < hmm->state_num ; j++)
			fprintf(stderr, "%f ", B[i][j]);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
#endif

#ifdef DEBUG
		/* test alpha */
		for (int i = 0 ; i < nr_state ; i++) {
			for (int t = 0 ; t < nr_time ; t++) {
				fprintf(stderr, "%.1e ", a[t][i]);
			}
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "\n");
		/* test beta */
		for (int i = 0 ; i < nr_state ; i++) {
			for (int t = 0 ; t < nr_time ; t++) {
				fprintf(stderr, "%.1e ", b[t][i]);
			}
			fprintf(stderr, "\n");
		}
		break;
#endif

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


#ifdef DEBUG
	fprintf(stderr, "tmp_prob = %.5e\nModel #%d after test_core:\nPi:\n",tmp_prob, k);
	for (int i = 0 ; i < hmms[k].state_num ; i++)
		fprintf(stderr, "%.5lf ", Pi[i]);
	fprintf(stderr, "\n\nA:\n");

	for (int i = 0 ; i < hmms[k].state_num ; i++) {
		for (int j = 0 ; j < hmms[k].state_num ; j++)
			fprintf(stderr, "%.5lf ", A[i][j]);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\nB:\n");
		
	for (int i = 0 ; i < hmms[k].observ_num ; i++) {
		for (int j = 0 ; j < hmms[k].state_num ; j++)
			fprintf(stderr, "%.5lf ", B[i][j]);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
#endif
