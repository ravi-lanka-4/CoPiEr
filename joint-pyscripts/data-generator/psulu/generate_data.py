import yaml, os
import tempfile
import numpy as np
import glob, re
from createObst import main as createobst
from createObst import firstPassCommandLine as createobstArgs

OBSTACLES = False
BASEFOLDER = os.environ['COTRAIN_DATA'] + '/psulu/'
PREFIX = BASEFOLDER
#if OBSTACLES:
#    PREFIX = BASEFOLDER + './obst'
#else:
#    PREFIX = BASEFOLDER + './ts/'

CONFIG='./config/param.yaml'
LP_OUT= PREFIX + '/lp_files/'
FEAT_OUT= PREFIX + '/feats/'
ENV_OUT= PREFIX + '/env/'
PLOT_OUT= PREFIX + '/plot/'
SOL_OUT= PREFIX + '/sol/'

def createLP(idx, totalcount, write=False):
  template = yaml.load(open(CONFIG, 'r'))
  feasible = True
  args = createobstArgs()
  createobst(args)

  # Create the obstacle map for this instance
  os.system('python createObst.py -n 12')

  j = 0
  probsize = 15

  while probsize < 16:
    with tempfile.NamedTemporaryFile() as temp, tempfile.NamedTemporaryFile() as enviF:
      # Creates the input lp file
      from PuLPpSulu import main

      # Generate a single obstacle
      #if (j != 0) and OBSTACLES:
      #  os.system('python createObst.py -n 1 -o %s'%enviF.name)
      #  os.system('echo "    obs_%d:" >> ./config/newEnvi.yaml'%probsize)
      #  os.system('cat %s | tail -n 6 >> ./config/newEnvi.yaml'%enviF.name)

      # Move the sample file
      template['environment'] = "[%s]"%('./config/newEnvi.yaml')
      if OBSTACLES:
        template['waypoints'] = 10
      else:
        template['waypoints'] = probsize

      # Write input file
      yaml.dump(template, temp)

      # Solve pSulu for the input
      cfeasible = main(temp.name)
      feasible = feasible and cfeasible

      if not feasible:
          # Remove the last few lines in the new Envi file
          #os.system('head -n -6 ./config/newEnvi.yaml')
          #continue
          return False

      if totalcount < 800:
        outDir = LP_OUT + '/train'
        solDir = SOL_OUT + '/train'
        envDir = ENV_OUT + '/train'
        featDir = FEAT_OUT + '/train'
        pltDir = PLOT_OUT + '/train'
      elif (totalcount >= 800) and (totalcount < 1000):
        outDir = LP_OUT + '/valid'
        solDir = SOL_OUT + '/valid'
        envDir = ENV_OUT + '/valid'
        featDir = FEAT_OUT + '/valid'
        pltDir = PLOT_OUT + '/valid'
      else:
        outDir = LP_OUT + '/test'
        solDir = SOL_OUT + '/test'
        envDir = ENV_OUT + '/test'
        featDir = FEAT_OUT + '/test'
        pltDir = PLOT_OUT + '/test'

      # Check existence of the directory
      if not os.path.exists(outDir):
         os.makedirs(outDir)
      if not os.path.exists(solDir):
         os.makedirs(solDir)
      if not os.path.exists(pltDir):
         os.makedirs(pltDir)
      if not os.path.exists(envDir):
         os.makedirs(envDir)
      if not os.path.exists(featDir):
         os.makedirs(featDir)

      # Create solution
      csol = solDir + '/input%d.sol'%(totalcount)
      os.system('gurobi_cl ResultFile=%s MILP1.lp'%csol)

      # Move MILP to output
      os.system('mv MILP1.lp %s'%(outDir + '/input%d.lp'%(totalcount)))
      os.system('mv feats.pkl %s'%(featDir + '/feats%d.pkl'%(totalcount)))
      os.system('mv output/output_plot.png %s'%(pltDir + '/plt%d.png'%(totalcount)))
      os.system('cp config/newEnvi.yaml %s'%(envDir + '/envi%d.yaml'%(totalcount)))
      os.system('[ -e file ] && rm MILP*')

    print('%s/input%d.lp'%(outDir, totalcount))
    print('Problem Size: %d'%probsize)
    probsize = probsize + 1
    j = j + 1

  return feasible

if __name__ == '__main__':
    cnt = 0
    for i in range(1, 10000):
        feasible = createLP(i, cnt)
        if feasible:
           cnt = cnt + 1

        if cnt == 1500:
           break
