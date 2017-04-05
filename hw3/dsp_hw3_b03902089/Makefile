# The following two variable will be commandline determined by TA
# For testing, you could uncomment them.
SRIPATH ?= /Users/Qhan/dsp/hw3/srilm-1.5.10
MACHINE_TYPE ?= macosx
LM ?= bigram.lm

CXX = g++
CXXFLAGS = -O3 -I$(SRIPATH)/include -w
vpath lib%.a $(SRIPATH)/lib/$(MACHINE_TYPE)

TARGET = mydisambig
SRC = mydisambig.cpp
OBJ = $(SRC:.cpp=.o)
TO = ZhuYin-Big5.map
FROM = Big5-ZhuYin.map
.PHONY: all clean map run

all: $(TARGET)

$(TARGET): $(OBJ) -loolm -ldstruct -lmisc
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

run:
	@for i in $(shell seq 1 10) ; do \
		echo "Running $$i.txt"; \
		time ./mydisambig -text testdata/$$i.txt -map $(TO) -lm $(LM) -order 2 > result2/$$i.txt; \
	done;

map:
	@echo "Mapping!"
	@python3 mapping.py $(FROM) $(TO)

clean:
	$(RM) $(OBJ) $(TARGET)

