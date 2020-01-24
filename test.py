import random

with open('input', 'w+') as f:
	f.write("100000\n")

	for i in range(100000):
		x = random.randrange(1, 100, 1)
		f.write(str(x) + " ")
	f.write('\n')