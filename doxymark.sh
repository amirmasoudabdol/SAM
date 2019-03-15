for manpage in doc/man/man3/*.3
do
  # echo $manpage
  markdownfile="class_"$(basename "$manpage" .3)".md"
  # echo $markdownfile
  pandoc -f man -t markdown -s $manpage -o doc/api/$markdownfile
done