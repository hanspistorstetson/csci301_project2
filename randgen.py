import random

N = 100


l = [random.randint(0, 1000) for r in xrange(N)]

with open('input.txt', 'w') as fp:
    fp.write(str(len(l)) + "\n")
    fp.write(str(" ".join(str(x) for x in l)));

