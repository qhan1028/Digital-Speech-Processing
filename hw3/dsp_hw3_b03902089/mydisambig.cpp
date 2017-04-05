#include <stdio.h>
#include <stdlib.h>
#include "Ngram.h"
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstddef>
#include <algorithm>
#include <map>

using namespace std;

struct Word{
	string str;
	double prob;
	int pre;
	Word(string a)
	{
		str = a;
		prob = 0;
		pre = 0;
	}
};

typedef vector<Word>::iterator VW_iter;
typedef vector<string>::iterator VS_iter;

// P(w2 | w1)
double getBigramProb(const char *w1, const char *w2, Vocab& voc, Ngram& lmm)
{
	VocabIndex wid1 = voc.getIndex(w1);
	VocabIndex wid2 = voc.getIndex(w2);

	if(wid1 == Vocab_None)  //OOV
		wid1 = voc.getIndex(Vocab_Unknown);
	if(wid2 == Vocab_None)  //OOV
		wid2 = voc.getIndex(Vocab_Unknown);

	VocabIndex context[] = { wid1, Vocab_None };
	return lmm.wordProb( wid2, context);
}

vector<string> split(string A)
{
	vector<string> word_list;
	int L = A.length();

	// split chinese charaters and push into vector
	for(int j = 0; j < A.length(); j++)
	{
		if (A[j] == ' ') continue;
		else {
			string a = A.substr(j, 2);
			word_list.push_back(a);
			j++;
		}
	}
	return word_list;
}

vector<Word> split_to_word(string A)
{
	vector<Word> word_list;
	int L = A.length();

	// split chinese charaters and push into vector
	for(int j = 0; j < A.length(); j++)
	{
		if (A[j] == ' ') continue;
		else {
			string a = A.substr(j, 2);
			word_list.push_back(Word(a));
			j++;
		}
	}
	return word_list;
}

int main(int argc, char* argv[])
{
	char* maps;
	char* texts;
	char* lms;
	char* orders;
	for (int i = 1; i <= 8; i += 2)
	{
		if(strcmp(argv[i], "-map") == 0)
			maps = (argv[i + 1]);
		else if(strcmp(argv[i], "-text") == 0)
			texts = argv[i + 1];
		else if(strcmp(argv[i], "-lm") == 0)
			lms = argv[i + 1];
		else if(strcmp(argv[i], "-order") == 0)
			orders = argv[i + 1];
		else 
			printf("known command\n");
	}

	ifstream map (maps, ifstream::in);
	ifstream text (texts, ifstream::in);
	ifstream lm (lms, ifstream::in);
	int order = atoi(orders);

	// use Ngram library
	Vocab voc;
	Ngram lmm(voc, 2);
	const char lm_name[] = "./bigram.lm";
	File lmFile(lm_name, "r");
	lmm.read(lmFile);
	lmFile.close();

	// read word map (word options)
	string word_string;
	std::map <string, vector<Word> > M;
	while (getline(map, word_string)) 
	{
		string word = word_string.substr(0, 2);
		M[word] = split_to_word(word_string);
	}
	
	vector<vector<Word> > V;		// full sentence
	string sentence;
	while(getline(text, sentence))
	{
		V.clear();
		vector<string> s = split(sentence);
		for(VS_iter word = s.begin(); word != s.end(); word++)
		{
			// find mapping from ZhuYin to chinese characters
			V.push_back(M.find(*word)->second);
		}

		vector<Word> end_str;
		end_str.push_back(Word("</s>"));
		V.push_back(end_str);

		// find first word P
		for(VW_iter word = V.begin()->begin(); word != V.begin()->end(); word++)
			word->prob = getBigramProb("", word->str.c_str(), voc, lmm);

		// Viterbi
		for(vector<vector<Word> >::iterator vt = V.begin() + 1; vt != V.end(); vt++)
		{
			//printf("w1 size = %d, w2 size = %d\n", (vt-1)->size(), vt->size());
			int fastnode = 0;
			int threshold = 0;
			for(VW_iter w2 = vt->begin(); w2 != vt->end(); w2++)
			{
				double bestprob = -2147483647;
				int bestnode = 0;
				int k = 0;
				for(VW_iter w1 = (vt - 1)->begin(); w1 != ((vt - 1)->end()); w1++)
				{
					double tmp_prob = ((getBigramProb(w1->str.c_str(), w2->str.c_str(), voc, lmm))
									+ (w1->prob));		// P(w2 | w1)
					if(tmp_prob >= bestprob)
					{
						bestprob = tmp_prob;
						bestnode = k;
					}
					k++;
				}
				//printf("w2: best node = %d, bestprob = %f\n", bestnode, bestprob);
				w2->pre = bestnode;
				w2->prob = bestprob;
			}
		}


		vector<string> result;
		int tmpend = 0;
		for(vector<vector<Word> >::iterator vt = (V.end() - 1); vt >= V.begin(); vt--)
		{
			result.push_back(vt->at(tmpend).str);
			tmpend = vt->at(tmpend).pre;
		}

		printf("<s> ");
		for(VS_iter word = result.end()-1; word != result.begin()-1; word--)
			printf("%s ", word->c_str());
		printf("\n");
	}
}
