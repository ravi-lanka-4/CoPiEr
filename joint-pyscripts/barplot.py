import matplotlib.pyplot as plt
import numpy as np

yy = [[27.9353125,  23.2829375,  23.2175625], 
      [15.541,      15.553625,   12.9695625], 
      [16.4274375,  17.5659375,  13.941875], 
      [22.717,      20.9888125,  17.8004375]]

for k, i in enumerate(range(len(yy))):
  for x, y, c, l in zip(i + 0.15*np.array(range(3)), yy[i], ['r', 'g', 'b'], ['DAgger', 'DAgger self-supervision', 'Co-training']):
    if k == 0:
      plt.bar(x, y, width=0.15, color=c, label=l)
    else:
      plt.bar(x, y, width=0.15, color=c)

plt.legend()
plt.xticks(np.array(range(len(yy))) + 0.2, ['15\n(1080)', '17\n(1224)', '19\n(1368)', '21\n(1512)'])

plt.xlabel('Time steps')
plt.ylabel('Optimality Gap')

plt.savefig('out.png')

