# for manpage in doc/man/man3/*.3
# do
#   # echo $manpage
#   markdownfile="class_"$(basename "$manpage" .3)".md"
#   # echo $markdownfile
#   pandoc -f man -t markdown -M parent=API -s $manpage -o doc/api/$markdownfile
# done


for markdownfile in doc/api/*.md
do
	echo $markdownfile
	lines="$(grep '.*\\$' $markdownfile)"
	for l in $lines
	do
		echo $l
	done
done