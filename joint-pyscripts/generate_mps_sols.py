import argparse
import os, sys, glob

run = "scip-dagger/bin/scipdagger -m 0.01 -r 1 -s scip-dagger/scip.set -f {0} -o {1} --sol {2} --nodepru oracle --nodesel oracle"

def proc(inpF, csol):
  '''
  Processing to generate the solution
  '''
  outsol = csol.replace('.sol', '.mps.sol')
  print(run.format(inpF, csol, outsol))
  os.system(run.format(inpF, csol, outsol))
  return

def main(cFolder):
    for suffix in ['unlabeled', 'labeled', 'valid', 'test']:
      inpFolder = cFolder + '/%s'%suffix
      solFolder = inpFolder.replace('mps', 'sol')
      for inpF in glob.glob(inpFolder + '/*.mps'):
          solF = solFolder + '/' + os.path.basename(inpF).replace('.mps', '.sol')
          proc(inpF, solF)

if __name__=='__main__':
  main(sys.argv[1])

