GCC=gcc
FLAGS=-O3 -lm
ITERATION=1000

all: 
	$(GCC) $(FLAGS) train.c -o train
	$(GCC) $(FLAGS) test.c -o test
	@$(GCC) $(FLAGS) accuracy.c -o accuracy

trn:
	 ./train $(ITERATION) model_init.txt seq_model_01.txt model_01.txt
	 ./train $(ITERATION) model_init.txt seq_model_02.txt model_02.txt
	 ./train $(ITERATION) model_init.txt seq_model_03.txt model_03.txt
	 ./train $(ITERATION) model_init.txt seq_model_04.txt model_04.txt
	 ./train $(ITERATION) model_init.txt seq_model_05.txt model_05.txt

tst:
	./test modellist.txt testing_data1.txt result1.txt
	./test modellist.txt testing_data2.txt result2.txt

acc:
	./accuracy result1.txt testing_answer.txt
	@echo "accuracy:"  `cat acc.txt`

clean:
	rm -rf train test accuracy

