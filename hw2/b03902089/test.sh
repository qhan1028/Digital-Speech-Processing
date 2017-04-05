min_state=$1
max_state=$2
min_iter=$3
max_iter=$4

proto=lib/proto
proto_tmp=lib/proto_
result=result/accuracy
front=03_training_front.sh
end=03_training_end.sh

# clear result
echo -n > $result

# save partial code of 03_training.sh
head -n 17 03_training.sh > $front
tail -n 44 03_training.sh > $end

for (( i=$min_state ; i < $max_state+1 ; i=i+1 )); do

	file=$proto_tmp$i
	touch $file

	# print initial data
	echo '~o <VECSIZE> 39 <MFCC_Z_E_D_A>' >> $file
	echo '~h "proto"' >> $file
	echo '<BeginHMM>' >> $file
	echo '<NumStates>' $i >> $file

	# print states
	for (( j=2 ; j < $i ; j=j+1 )); do
		
		echo '<State>' $j >> $file
		echo '<Mean> 39' >> $file
		for k in {1..39}; do
			echo -n '0.0 ' >> $file
		done
		echo >> $file

		echo '<Variance> 39' >> $file
		for k in {1..39}; do
			echo -n '1.0 ' >> $file
		done
		echo >> $file

	done

	# print transition matrix
	echo '<TransP>' $i >> $file

	echo -n '0.0 1.0 ' >> $file
	for (( k=3 ; k < $i+1 ; k=k+1 )); do
		echo -n '0.0 ' >> $file
	done
	echo >> $file

	for (( j=2 ; j < $i ; j=j+1 )); do
		
		for (( k=1 ; k < $i+1 ; k=k+1 )); do
			
			if [ $k -eq $j ]; then
				echo -n '0.5 ' >> $file 
			elif [ $k -eq `expr $j + 1` ]; then
				echo -n '0.5 ' >> $file
			else
				echo -n '0.0 ' >> $file
			fi

		done
		echo >> $file

	done

	for (( k=1 ; k < $i+1 ; k=k+1 )); do
		echo -n '0.0 ' >> $file
	done
	echo >> $file

	echo '<EndHMM>' >> $file

	cat $file > $proto 
	rm $file

	# start testing with different iterations
	for (( j=$min_iter ; j < $max_iter+1 ; j=j+1 )); do

		k=`expr $j - 1`
		cat $front > 03_training.sh
		printf "for i in {0..%d}\n" $k >> 03_training.sh
		cat $end >> 03_training.sh

#		echo "Number of State:" $i >> $result
#		echo "Iterations:" $j >> $result

		./00_clean_all.sh
		./01_run_HCopy.sh
		./02_run_HCompV.sh
		./03_training.sh
		./04_testing.sh

	done

done

rm -rf $front $end

# print result
cat $result 
