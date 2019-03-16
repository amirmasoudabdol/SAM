# for manpage in doc/man/man3/*.3
# do
#   # echo $manpage
#   markdownfile="class_"$(basename "$manpage" .3)".md"
#   # echo $markdownfile
#   pandoc -f man -t markdown -M parent=API -s $manpage -o doc/api/$markdownfile
# done

for xmlfile in doc/docbook/class_*.xml
do
  # echo $manpage
  markdownfile=$(basename "$xmlfile" .xml)".md"
  # echo $markdownfile
  pandoc -f docbook -t markdown -M parent=API -s $xmlfile -o doc/api/$markdownfile

  sed -i -e 's/{#.*//g' doc/api/$markdownfile

  sed -i -e 's/hsrc/h\
  \
src/g' doc/api/$markdownfile
  
  pandoc -s doc/api/$markdownfile --filter delink.hs -o doc/api/$markdownfile
done


# for markdownfile in doc/api/*.md
# do
# 	echo $markdownfile
# 	lines="$(grep '.*\\$' $markdownfile)"
# 	for l in $lines
# 	do
# 		echo $l
# 	done
# done