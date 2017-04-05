./separator_big5.pl < corpus.txt > corpus_seg.txt
./srilm-1.5.10/bin/macosx/ngram-count -text corpus_seg.txt -write lm.cnt -order 2
./srilm-1.5.10/bin/macosx/ngram-count -read lm.cnt -lm bigram.lm -unk -order 2

