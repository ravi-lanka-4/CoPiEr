import argparse, glob
import os
from getLog import procLog
import numpy as np

move = lambda src, fname, dst:  os.system('cp %s/%s %s/'%(src, fname, dst))

def readSols(src):
  '''
  Read bunch of sols together
  '''
  objMap = {}
  for cF in glob.glob(src + '/*.sol'):
    name = os.path.basename(cF)
    with open(cF, 'r') as F:
        data = F.readlines()

    if 'objective' in data[0]:
      obj = data[0].strip('objective value: ') 
    elif 'no solution' in data[0]:
      obj = 10000
    elif 'Objective' in data[1]:
      obj = data[1].strip('# Objective value = ') 
    else:
      raise Exception('Expected objective value in the first line')

    objMap[name] = obj

  return objMap

def main(src1, src2, dst):
  objSrc1 = readSols(src1)
  objSrc2 = readSols(src2)
  allF = np.unique(objSrc1.keys() + objSrc2.keys())
  print(dst)
  os.system('mkdir -p %s'%dst)

  for cF in allF:
    logF = cF.replace('.sol', '.log')
    if cF in objSrc1:
      obj1 = float(objSrc1[cF])
    else:
      obj1 = 10000

    if cF in objSrc2:
      obj2 = float(objSrc2[cF])
    else:
      obj2 = 10000

    if (obj1 == 10000) and (obj2 == 10000):
        continue

    print('Comparing %f and %f'%(obj1, obj2))
    if obj1 < obj2:
        # move from src1
        move(src1, cF, dst)
        move(src1, logF, dst)
    else:
        # move from src2
        move(src2, cF, dst)
        move(src2, logF, dst)


def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--s1', type=str, default="/Users/subrahma/proj/maverick-setup/co-training/scratch/Users/subrahma/proj/maverick-setup/co-training/data/psulu_test/lpfiles/unlabeled/nn_psulu_0.25_cotrain_search_lp_0",
                       help='Source 1', dest='src1')
  parser.add_argument('--s2', type=str, default="/Users/subrahma/proj/maverick-setup/co-training/scratch/Users/subrahma/proj/maverick-setup/co-training/data/psulu_test/lpfiles/unlabeled/nn_psulu_0.25_cotrain_search_mps_0",
                       help='Source 2', dest='src2')
  parser.add_argument('--d', type=str, 
                       help='destination', default="/Users/subrahma/proj/maverick-setup/co-training/scratch/Users/subrahma/proj/maverick-setup/co-training/data/psulu_test/sol/train/nn_psulu_0.25_cotrain_search_lp_0", dest='dst')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  if args.src1 and args.src2 and args.dst:
    main(args.src1, args.src2, args.dst)

