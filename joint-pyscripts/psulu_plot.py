import os, sys, argparse
import numpy as np
import matplotlib.pyplot as plt

HOME=os.environ['COTRAIN_HOME']
DATA=os.environ['COTRAIN_DATA']
SCRATCH=os.environ['COTRAIN_SCRATCH']
sys.path.insert(0, os.path.join(HOME, './joint-pyscripts/data-generator/psulu/'))

from pSulu_api import ObstacleMap
from psulu_compare import readSol

fig = plt.figure()

def plot(env, sol, c='g', c1='k', clabel=None):
  ## Plot obstacles
  if env:
    for corners in env.obstVert:
      x = corners[:,0]
      y = corners[:,1]
      plt.fill(x, y, color='black')

  # Read way points from solution
  print('Reading %s'%sol)
  allVars, numSteps = readSol(sol)
  if allVars == None:
      # No feasible solution
      return

  wp = np.zeros((numSteps, 2))
  for var, val in allVars.iteritems():
    if 'x_' in var:
      var = var.replace('x_', '')
      (step, side) = var.split('_')
      if int(side) < 2:
          wp[int(step), int(side)] = val

  # plot the way points
  for i, (pt1, pt2) in enumerate(zip(wp[:-1], wp[1:])):
    if i == 0:
      plt.plot([pt1[0], pt2[0]], [pt1[1], pt2[1]], c=c, label=clabel)
    else:
      plt.plot([pt1[0], pt2[0]], [pt1[1], pt2[1]], c=c)
    plt.scatter(pt1[0], pt1[1], c=c1)
    plt.scatter(pt2[0], pt2[1], c=c1)

  plt.legend()
  return

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--e', type=str, default='psulu/env/test/envi1001.yaml',
                       help='Environment file', dest='envF')
  parser.add_argument('--s1', type=str, default='12obst_15ts/pred/test/input1001.scip.sol',
                       help='first full solution', dest='sol1')
  parser.add_argument('--s2', type=str, default='12obst_15ts/pred/test/input1001.policy.sol',
                       help='second full solution', dest='sol2')
  parser.add_argument('--s3', type=str, default='12obst_15ts/pred/test/input1001.new.sol',
                       help='third solution', dest='sol3')
  parser.add_argument('--o', type=str, default='12obst_15ts/pred/test/input1001.plt.png',
                       help='output solution', dest='outF')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  envF = os.path.join(DATA, args.envF)
  sol1 = args.sol1
  sol2 = args.sol2
  #sol1 = os.path.join(SCRATCH, args.sol1)
  #sol2 = os.path.join(SCRATCH, args.sol2)
  #sol3 = os.path.join(SCRATCH, args.sol3)
  #outF = os.path.join(SCRATCH, args.outF)
  outF = args.outF

  # Read environment file
  env = ObstacleMap(envF) 
  plot(env, sol1, clabel='SCIP')
  #plot(None, sol3, c='y', c1='c', clabel='Spatial')
  plot(None, sol2, c='r', c1='b', clabel='Policy Fix 1')

  print('Writing: %s'%outF)
  plt.savefig(outF)
  plt.clf()

