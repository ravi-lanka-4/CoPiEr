import argparse, os
from psulu_compare import readSol, createProb
from psulu_compare import writeSol, getVarMap, partialSolObj

def main(lpF, envF, inpSol, outSol, lambda_=0.05):
  os.system('mkdir -p %s'%(os.path.dirname(outSol)))
  vals, nSteps = readSol(inpSol)
  if not isinstance(nSteps, int):
      return

  prob = createProb(envF, nSteps-1)
  varMap, sideMap, varIDMap = getVarMap(prob)
  objpart, binaryVars = partialSolObj(prob, inpSol, varMap, sideMap, lambda_)
  writeSol(binaryVars, outSol)
  return

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--i', type=str, default='/Users/subrahma/proj/maverick-setup/co-training/data/psulu_test/lpfiles/unlabeled/input4.lp',
                       help='Input LP File', dest='inpF')
  parser.add_argument('--e', type=str, default='/Users/subrahma/proj/maverick-setup/co-training/data/psulu_test/env/unlabeled/envi4.yaml',
                       help='Environment file', dest='envF')
  parser.add_argument('--s', type=str, 
                       default='/Users/subrahma/proj/maverick-setup/co-training/scratch/Users/subrahma/proj/maverick-setup/co-training/data/psulu_test/mps/unlabeled/0/input4.sol',
                       help='quad solution', dest='inpSol')
  parser.add_argument('--o', type=str, 
                       default='/Users/subrahma/proj/maverick-setup/co-training/scratch/Users/subrahma/proj/maverick-setup/co-training/data/psulu_test/quad_to_lp/unlabeled/0/input4.sol',
                       help='lp solution; will contain only binary variables', dest='outSol')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  main(args.inpF, args.envF, args.inpSol, args.outSol)

