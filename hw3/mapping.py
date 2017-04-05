import sys
import string

mapping = dict()

for line in open('Big5-ZhuYin.map', 'r', encoding = 'big5-hkscs'):
	line = line.replace(" ", "")
	c = line[0]
	z = line[1:].split('/')
	if c not in mapping:
		mapping[c] = set()
		mapping[c].add(c)

	for i in range(len(z)):
		if z[i][0] not in mapping:
			mapping[z[i][0]] = set()
			mapping[z[i][0]].add(z[i][0])
		else:
			mapping[z[i][0]].add(c)

out = open('ZhuYin-Big5.map', 'w', encoding = 'big5-hkscs', newline=None)
for c in mapping:
	print (c, end=" ", file=out)
	for v in mapping[c]:
		print (v, end=" ", file=out)
	print("", file=out)
