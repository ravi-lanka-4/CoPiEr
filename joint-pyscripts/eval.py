import numpy as np
import sys, os

prefix = sys.argv[1]
if len(sys.argv) > 2:
    flag = True
else:
    flag = False

cmd_ = lambda folder, x: 'find %s/ -name "*.log" | grep -v "full.log" | sort | xargs grep -Rs "Gap                :" {} > %d.txt'%(folder, x)

num = 0
for j in range(5):
  folder = prefix + '_%d'%j
  print(folder)
  if os.path.isdir(folder):
    os.system(cmd_(folder, int(j)))
    num = num + 1
  else:
    break

alldata = []
outF = ['%d.txt'%j for j in range(num)]
for fname in outF:
  data = []
  with open(fname, 'r') as F:
    cdata = F.readlines()
    for cc in cdata:
      try:
        val = float(cc.split(': ')[1].strip('\n').strip('%'))
      except:
        val = 300
      data.append(val)
    alldata.append(data)

if flag:
    idx = []
    alldata = np.array(alldata)
    for cdata in alldata:
      cdata = np.array(cdata)
      cidx = np.where(cdata > 200)[0]
      idx.extend(cidx)

    for i in range(len(alldata)):
      alldata[i][idx] = 0

mean_ = []
for data in alldata:
    mean_.append(np.mean(data))

print(mean_)

