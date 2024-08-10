
if [[ $# -gt 0 ]]; then
    max_score=$1
else
    max_score=100
fi
# echo $max_score

if [[ $# -gt 1 ]]; then
    max_stid=$2
else
    max_stid=5
fi
# echo $max_stid




#Array for student mark
stMark=(0 0 0 0 0 0 0 0 0)
stId=(1805121 1805122 1805123 1805124 1805125 1805126 1805127 1805128 1805129)

var=$(ls ./Submissions/) 

count=0
checker=0 #for 2-3 files in a folder

for str in ${var[@]}; do
	myArray[$count]=$str  #folder names will be stored in myArray
	count=$(($count+1))
done

# echo "${#myArray[@]}"
touch dummy.txt
for ((i = 0, j = 0; i < $max_stid; i++)); do
	if [[ ${stId[$i]} = ${myArray[$j]} ]] ; then
		for arg in $(find ./Submissions/${myArray[$j]} -type f) ; do
    		if [[ $arg = *".sh" ]] && [[ ${arg: -10: -3} = ${myArray[$j]} ]]  ; then
				checker=1
				if [[ $arg != $0 ]]; then
            		bash $arg > a.txt
            		diff -w  AcceptedOutput.txt a.txt > dummy.txt
            		counter=0
            		while read -r line; do
                		if [[ ${line:0:1} = "<" ]] ||  [[ ${line:0:1} = ">" ]]; then
                    		counter=$(($counter + 1 ))
                		fi
        	    	done <dummy.txt
            		counter=$(($counter*5))
            		if [[ $counter -ge $max_score ]]; then
                		stMark[$i]=0
            		else
                		stMark[$i]=$(($max_score-$counter)) 
            		fi
				fi
			else
				if [[ $checker -ne 1 ]]; then
					stMark[$i]=0
				fi
			fi
		done
		j=$(($j+1))
	else
		stMark[$i]=0
	fi
done


#For Copy Check
touch dummy2.txt
touch dummy3.txt
for (( i=1; i <= $max_stid; i++ )); do
	if ls ./Submissions/ | grep 180512$i >dummy2.txt && [[ ${stMark[$(($i-1))]} -ne 0 ]] ; then
		in1=./Submissions/180512$i/180512$i.sh
		for (( j=1; j <= $max_stid; j++ )); do
			if ls ./Submissions/ | grep 180512$j >dummy2.txt && [[ ${stMark[$(($j-1))]} -ne 0 ]] && [[ i -ne j ]] ; then
				in2=./Submissions/180512$j/180512$j.sh	
				diff -Z -B  $in1 $in2 >dummy3.txt
				if [ $? -eq 0 ]; then
					if [[ ${stMark[$(($i-1))]} -gt 0 ]]; then
						var2=${stMark[$(($i-1))]}
						var2=$(($var2 * -1))
						stMark[$(($i-1))]=$var2
						# echo ${stMark[$(($i-1))]} 
					fi
					if [[ ${stMark[$(($j-1))]} -gt 0 ]]; then
						var2=${stMark[$(($j-1))]}
						var2=$(($var2 * -1))
						stMark[$(($j-1))]=$var2
						# echo ${stMark[$(($j-1))]} 
					fi
					break
				fi
			fi
		done
	fi
done 

# #accessing using indices
# for i in ${!stMark[@]}; do
# 	echo "element $i is ${stMark[$i]}"
# done



#Output in CSV
echo "student_id,score">output.csv

for (( i=0; i < $max_stid; i++ )); do
	echo "${stId[$i]} ,${stMark[$i]} ">>output.csv
done


rm ./a.txt
rm ./dummy.txt
rm ./dummy2.txt
rm ./dummy3.txt



#------------various IF syntax form--------------------

# if [ $val = "abc" ]; then #Older version of sh
# if [[ $val = "abc" ]]; then  #Preferable for bash and, or kora easy
# if (($val < 10)); then  #Use for Arithmatic
# if grep i a.cpp; then #Command thakbe, no bracket, true hobe jokhon command theke value 0 ashbe, 1-> false