LIST=$(ls traces/)
for file in $LIST
do
	echo -n "$file ->"
	./mdriver2 -a -f traces/$file
	echo 
done
