if [[ "$#" -eq '1' && "$1" = '-h' ]]
then
	echo "bash script.sh [-hga]";
	echo "Options";
	echo -e "\t-h\tUsage";
	echo -e "\t-a\tRun with tracefiles/*";
	echo -e "\t-g\tShow only perf-index";
	echo -e "\t-t\tShow only sum of all perf-index";
	echo -e "\t-ag\tShow only perf-index when run with tracefiles/";
	echo -e "\t-at\tShow only sum of all perf-index when run with tracefiles/";
	exit 0
fi

FOLDER='traces'
GRADER='FALSE'
MAKE_SUM='FALSE'
if [[ $# -eq '1' && "$1" = '-a' ]]
then
	FOLDER='tracefiles'
elif [[ $# -eq '1' && "$1" = '-g' ]]
then
	GRADER='TRUE'
elif [[ $# -eq '1' && "$1" = '-t' ]]
then
	GRADER='TRUE'
	MAKE_SUM='TRUE'
elif [[ $# -eq '1' && "$1" = '-ag' ]]
then
	GRADER='TRUE'
	FOLDER='tracefiles'
elif [[ $# -eq '1' && "$1" = '-at' ]]
then
	GRADER='TRUE'
	FOLDER='tracefiles'
	MAKE_SUM='TRUE'
fi

LIST=$(ls "$FOLDER/")

TOTAL=0
for file in $LIST
do
	if [[ $GRADER = "TRUE" ]]
	then
		val=$(./mdriver2 -ag -f "$FOLDER/$file" | tail -1 | awk 'BEGIN{FS=":"}{print $2}')
		if [[ $MAKE_SUM = 'FALSE' ]]
		then
			echo $val
		fi
		TOTAL=$((TOTAL+val))
	else
		echo -n "$file ->"
		./mdriver2 -a -f "$FOLDER/$file"
		echo
	fi	

done

if [[ $MAKE_SUM = 'TRUE' ]]
then
	echo $TOTAL
fi
