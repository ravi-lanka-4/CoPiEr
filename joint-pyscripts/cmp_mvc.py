import argparse, glob
import os
from getLog import procLog
import numpy as np

INFEASIBLE=10000

def readSol(solF):
  name = os.path.basename(cF)
  with open(cF, 'r') as F:
      data = F.readlines()

  obj = INFEASIBLE
  if 'objective' in data[0]:
    obj = data[0].strip('objective value: ') 
  elif 'no solution' in data[0]:
    obj = INFEASIBLE
  elif 'Objective' in data[1]:
    obj = data[1].strip('# Objective value = ') 
  else:
    raise Exception('Expected objective value in the first line')

  return obj

def main(sol1, sol2, dst, threshold=5):
  os.system('mkdir -p %s'%dst)

  src1sols = {}
  for solF in glob.glob(sol1 + '/*.sol'):
    fname = os.path.basename(solF)
    src1sols[fname] = self.readSol(solF)

  src2sols = {}
  for solF in glob.glob(sol2 + '/*.sol'):
    fname = os.path.basename(solF)
    src2sols[fname] = self.readSol(solF)

  fnames = set(src2sols.keys() + src1sols.keys())
  for f in fnames:
    if not (f in src1sols):
      src1sols[f] = INFEASIBLE
    
    if not (f in src2sols):
      src2sols[f] = INFEASIBLE

    if (src1sols[f] == INFEASIBLE) and (src2sols[f] == INFEASIBLE):
      continue
    elif src1sols[f] <= src2sols[f] + threshold:
      # Use sol 1
      os.system('mv %s/%s %s/'%(sol1, f, dst)
    else:
      # Use sol 2
      os.system('mv %s/%s %s/'%(sol2, f, dst)

  return

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--s1', type=str, default=None,
                       help='Source 1', dest='src1')
  parser.add_argument('--s2', type=str, default=None,
                       help='Source 2', dest='src2')
  parser.add_argument('--d', type=str, default="", 
                       help='destination', dest='dst')
  parser.add_argument('--t', type=int, help='threshold', default=5, dest='threshold')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  if args.src1 and args.src2 and args.dst:
    main(args.src1, args.src2, args.dst, args.threshold, args.flag, args.threshold)

