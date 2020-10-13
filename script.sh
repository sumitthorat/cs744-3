if [[ "$#" -eq '1' && "$1" = '-h' ]]
then
	echo "bash script.sh [-hga]";
	echo "Options";
	echo -e "\t-h\tUsage";
	echo -e "\t-a\tRun with tracefiles/*";
	echo -e "\t-g\tShow only perf-index";
	exit 0
fi

FOLDER='traces'
GRADER='FALSE'

if [[ $# -eq '1' && "$1" = '-a' ]]
then
	FOLDER='tracefiles'
elif [[ $# -eq '1' && "$1" = '-g' ]]
then
	GRADER='TRUE'
elif [[ $# -eq '1' && "$1" = '-ag' ]]
then
	GRADER='TRUE'
	FOLDER='tracefiles'
fi

LIST=$(ls "$FOLDER/")

for file in $LIST
do
	if [[ $GRADER = "TRUE" ]]
	then
		./mdriver2 -ag -f "$FOLDER/$file" | tail -1 | awk 'BEGIN{FS=":"}{print $2}'
	else
		echo -n "$file ->"
		./mdriver2 -a -f "$FOLDER/$file"
		echo
	fi	

done
