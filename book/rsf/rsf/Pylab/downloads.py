from pylab import *
from numpy import *

down = '''
54
166
279
357
396
469
531
582
650
793
931
1081
1191
1285
1372
1467
1666
1841
1933
2042
2113
2240
2356
2543
2709
2867
2997
3150
3344
3735
4076
4348
4672
5074
5262
5456
5769
6025
6307
6669
7042
7396
7828
8179
8484
8970
9309
9579
9833
10300
10684
11023
11351
11807
12089
12427
12706
13038
13349
13751
14103
14806
15229
15579
15922
16275
'''

down = map(int,down.split())
ind = arange(len(down))
width=0.75

p1 = bar(ind,down,width)
ylabel('Downloads')
xticks(ind[::12]+width/2, 
       ('6/06','6/07','6/08','6/09','6/10','6/11','6/12'))
title('Cumulative Downloads')
#grid(True)

#show()
savefig('junk_py.eps')
