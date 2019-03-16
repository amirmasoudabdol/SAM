import glob

mfiles = glob.glob("doc/api/*.md")

for mfile in mfiles:
	f = open(mfile, 'r')
	lines = f.readlines()
	
	for i in range(len(lines)):
		s = str(lines[i])
		j = s.rfind("\\")
		if j != -1:
			lines[i] = "`" + lines[i][:j] + "`" + lines[i][j+1:]
		# print(l)


	# f.close()

	f = open(mfile, 'w')
	for l in lines:
		f.write(l)

	# f.close()