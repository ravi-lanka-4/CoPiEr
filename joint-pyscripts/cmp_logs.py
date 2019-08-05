import argparse, glob
import os
from getLog import procLog
import numpy as np

move = lambda src, fname, dst:  os.system('cp %s/%s %s/'%(src, fname, dst))

def readLogs(src, threshold=30, flag=0):
  '''
  Read bunch of sols together
  '''
  objMap = {}
  for cF in glob.glob(src + '/*.log'):
    name = os.path.basename(cF)
    dualgap = procLog(cF)
    if dualgap == None:
        dualgap = 10000
    elif dualgap > threshold:
        if flag != 0:
            dualgap = 10000

    objMap[name] = float(dualgap)

  return objMap

def main(src1, src2, dst, threshold, flag):
  objSrc1 = readLogs(src1, flag)
  objSrc2 = readLogs(src2, flag)
  allF = np.unique(objSrc1.keys() + objSrc2.keys())
  print(dst)
  os.system('mkdir -p %s'%dst)

  # Don't clear these
  #os.system('rm %s/*.sol %s/*.log'%(dst, dst))

  for cF in allF:
    solF = cF.replace('.log', '.sol')
    if cF in objSrc1:
      obj1 = float(objSrc1[cF])
    else:
      obj1 = 10000

    if cF in objSrc2:
      obj2 = float(objSrc2[cF])
    else:
      obj2 = 10000

    print('Comparing %f and %f'%(obj1, obj2))
    if (obj1 == 10000) and (obj2 == 10000):
        continue
    elif (obj1 - obj2) > threshold:
        # move from src2
        move(src2, solF, dst)
    else:
        move(src1, solF, dst)
        print('Very little knowledge gain; still not skipping')

    #else:
    #    # move from src2
    #    move(src2, cF, dst)
    #    move(src2, solF, dst)

  return


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
  parser.add_argument('--t', type=int, help='threshold', default=10, dest='threshold')
  parser.add_argument('--b', type=int, help='should threshold be used', default=0, dest='flag')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  if args.src1 and args.src2 and args.dst:
    main(args.src1, args.src2, args.dst, args.threshold, args.flag)

