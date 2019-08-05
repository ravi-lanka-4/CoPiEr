import argparse
import os, tempfile
import yaml as Y
import numpy as np
import pulp as P
import random, glob
from termcolor import colored

# Global setting
DATA=os.path.join(os.environ['COTRAIN_DATA'], 'psulu')
HOME=os.environ['COTRAIN_HOME']
SCRATCH=os.environ['COTRAIN_SCRATCH']

import sys
sys.path.insert(0, os.path.join(HOME, './joint-pyscripts/data-generator/psulu/'))
from PuLPpSulu import IRA

_sampleConfig = os.path.join(HOME, './joint-pyscripts/data-generator/psulu/config/param.yaml')

def getTimeSteps(lpF):
  with open(lpF, 'r') as F:
    data = F.readlines()
    x = []
    z_start = False
    
    # first read state vector variables
    for cline in data:
        if ('free' in cline) and ('x_' in cline):
          x.append(cline.strip('free '))

  return (len(x)/4) - 1

def createProb(envF, numSteps):
  assert os.path.isfile(envF), 'environment file does not exist'

  with tempfile.NamedTemporaryFile() as temp:
    with open(_sampleConfig, 'r') as F:
      data = Y.load(F)
      data['environment'] = '[%s]'%envF
      data['quad'] = False
      data['waypoints'] = numSteps
    
    # Create temporary yaml file
    Y.dump(data, temp)
    pSulu = IRA(temp.name)
    lpProb = pSulu.create()
  
  return lpProb

def getVarMap(prob):
  vars_ = {}
  side_ = {}
  varsID = {}
  for cntr in prob.constraints.keys():
    cntr_ = prob.constraints[cntr]
    for v1, v2 in prob.constraints[cntr].items():
      varsID[v1.name] = v1
      if 'constraint' in cntr:
        if 'x_' in v1.name:
          continue
        if v1 in vars_:
          vars_[v1].append(cntr)
        else:
          vars_[v1] = [cntr]
      elif 'z_' in cntr:
        if cntr in side_:
          side_[cntr].append(v1)
        else:
          side_[cntr] = [v1]
      else:
        break

  return vars_, side_, varsID

def readSol(sol):
  vals = {}
  numSteps = 0
  with open(sol, 'r') as F:
    data = F.readlines()
    for cline in data:
      if '#' == cline[0]:
          continue
      elif 'objective' in cline:
          continue
      elif 'no solution available' in cline:
          return None, None

      cdata = cline.split('\t')[0]
      cdata = cdata.rstrip(' ')
      cdata = cdata.split(' ')
      name_ = cdata[0]
      val = cdata[-1]
      vals[name_] = float(val)
      if 'x_' in name_:
          numSteps = numSteps + 1

  return vals, numSteps/4

def evalCnstr(vals, cnstr):
  lhs = cnstr.constant
  for items in cnstr.items():
    var = items[0]
    coeff = items[1]

    try:
      cval = vals[var.name]
      lhs = lhs + coeff*cval
    except Exception as e:
      # Possible z variable
      if 'z_' in var.name:
        lhs = lhs + 0 # zero assigned to the variable 
      else:
        raise Exception('This variables solution is expected')

  return np.clip(lhs, 0, None) # clip at zero

def computeDist(vals, nSteps):
  from scipy.spatial import distance
  
  allts = range(nSteps)
  dist = 0
  import pprint as P
  for prevts, currts in zip(allts[:-1], allts[1:]):
    prev = []
    curr = []
    for j in range(2):
      prevstr_ = 'x_%d_%d'%(prevts, j)
      currstr_ = 'x_%d_%d'%(currts, j)
      prev.append(vals[prevstr_])
      curr.append(vals[currstr_])

    dist = dist + distance.euclidean(prev, curr)

  dist = dist + distance.euclidean(curr, [1,1])
  return dist

def partialSolObj(prob, solF, varMap, sideMap, lambda_):
  '''
  Evaluates a partial solution -- specific to pSulu
  '''
  vals, nSteps = readSol(solF)
  binaryVars = {}
  binaryVars.update(vals)
  obj  = 0

  # Check violating constraints:
  for zcnstr, czvar in sideMap.iteritems():
    # Skip constraints on the last time step
    cline = zcnstr.replace('z_', '')
    cstep = int(cline.split('_')[0]) 
    if cstep == nSteps-1:
        continue

    # \sum_i czvar_i == 3
    canset = []
    allslack = []
    for zj in czvar:
      # Get constraints corresponding to the current z variable
      slack = []
      for cnstr in varMap[zj]:
        # check for each constraint
        cslack = evalCnstr(vals, prob.constraints[cnstr])
        slack.append(cslack)
        
      # Check if the variable can be set; 
      # this corresponds to checking if the corresopnding side can be chosen
      allslack.append(slack)
      slack = np.array(slack)
      idx = np.where(slack > 0)[0]
      canset.append(idx.shape[0] == 0)

    # Check if the set of constraints can be satisfied with partial solution
    idx = np.where(canset)[0]
    if idx.shape[0] > 0:
      # No violation of constraint
      # Add to update the solution
      cidx = np.random.choice(idx)
      cval = np.ones(4,)
      cval[cidx] = 0
    else:
      # Choose the best violated constraint and add it to the objective
      print('Voilating constraints %s'%(czvar[0].name[:-2]))
      cslack = np.min(np.sum(allslack, axis=1)) 
      obj = obj + cslack*lambda_

      # Set values to -1 so it can be handled on SCIP
      cval = -1*np.ones(4,)

    cval = dict([(z.name, v) for z, v in zip(czvar, cval)])
    binaryVars.update(cval)

  obj = obj + computeDist(vals, nSteps)

  # Remove continuous variables from binaryVars
  colored('Removing the continuous variables', 'green')
  #for key, val in vals.iteritems():
  #    del binaryVars[key]

  return obj, binaryVars

def completeSolObj(solF):
  '''
  Assumes that the solution is feasible
  '''
  vals, nSteps = readSol(solF)
  if nSteps != None:
    dist = computeDist(vals, nSteps)
  else:
    dist = 1000

  return dist

def writeSol(binVars, outF):
  '''
  Writes the solution from the binary variables for partial solution
  '''
  with open(outF, 'w') as solF:
    for name, val in binVars.iteritems():
      if 'z_' in name:
        str_ = '%s %d\n'%(name, val)
      else:
        str_ = '%s %f\n'%(name, val)
      solF.write(str_)

  print('New solution written to %s'%outF)
  return

def compare(lpF, envF, solfull, solpart, outF, newLp, lambda_):
  '''
  Compares two different solutions 
  Handles partial solution as well
  '''
  # Read a standard pSulu problem and replace the constraints

  numSteps = getTimeSteps(lpF)
  prob = createProb(envF, numSteps)
  varMap, sideMap, varIDMap = getVarMap(prob)

  # First the partial solution
  objpart, binaryVars = partialSolObj(prob, solpart, varMap, sideMap, lambda_)
  objfull = completeSolObj(solfull)

  # Update LP file and 
  for key, val in binaryVars.iteritems():
    if 'x_' in key:
      continue

    if val != -1:  
      prob.addConstraint(varIDMap[key] == int(val), name='new_%s'%key)

  # Write new lp
  if newLp:
     print('Writing Lp %s'%newLp)
     prob.writeLP(newLp)

  if lambda_ < 0:
    # Force partial solution
    print('full %f, part %f'%(objfull, objpart))
    colored('Using partial solution as requested', 'red')
    objpart = -1

  print('full %f, part %f'%(objfull, objpart))
  if objfull < objpart:
      print('New solution written to %s'%outF)
      os.system('cp %s %s'%(solfull, outF))
      return solfull, objfull, objpart
  else:
      writeSol(binaryVars, outF)
      return solpart, objfull, objpart

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--i', type=str, default='lpfiles/test/input1001.lp',
                       help='Input LP File', dest='inpF')
  parser.add_argument('--e', type=str, default='env/test/envi1001.yaml',
                       help='Environment file', dest='envF')
  parser.add_argument('--s', type=str, default='12obst_15ts/pred/test/input1001.scip.sol',
                       help='full solution', dest='sol1')
  parser.add_argument('--p', type=str, default='12obst_15ts/pred/test/input1001.part.sol',
                       help='partial solution', dest='sol2')
  parser.add_argument('--o', type=str, default='12obst_15ts/pred/test/input1001.new.sol',
                       help='new solution upon comparison', dest='newSol')
  parser.add_argument('--l', type=str, default=None,
                       help='lp formulation for the new problem', dest='newLp')
  parser.add_argument('--c', type=float, default=0.1,
                       help='Cost of constraint violation', dest='lambda_')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  inpF = os.path.join(DATA, args.inpF)
  envF = os.path.join(DATA, args.envF)
  solfull = os.path.join(SCRATCH, args.sol1)
  solpart = os.path.join(SCRATCH, args.sol2)
  outSol = os.path.join(SCRATCH, args.newSol)
  if args.newLp:
    outLp = os.path.join(SCRATCH, args.newLp)
  else:
    outLp = None
  lambda_ = args.lambda_
  
  if os.path.isfile(solpart) and os.path.isfile(solfull):
    compare(inpF, envF, solfull, solpart, outSol, outLp, lambda_)
  else:
    fullsolFolder = os.path.dirname(solfull)
    partsolFolder = os.path.dirname(solpart)
    lpFolder      = os.path.dirname(inpF)
    envFolder     = os.path.dirname(envF)

    real = []
    part = []
    print('Processing %s'%os.path.dirname(partsolFolder))
    for solpart in glob.glob(partsolFolder + '/*part.sol'):
      fname = os.path.basename(solpart).replace('.part', '')
      solfull = os.path.join(fullsolFolder, fname)
      outF = os.path.join(outSol, fname.replace('.sol', '.new.sol'))
      inpF = os.path.join(lpFolder, fname.replace('.sol', '.lp'))
      envF = os.path.join(envFolder, fname.replace('.sol', '.yaml'))
      envF = envF.replace('input', 'envi')

      print('Comparing %s and %s'%(os.path.basename(solfull), os.path.basename(solpart)))
      _, fullobj, partobj = compare(inpF, envF, solfull, solpart, outF, outLp, lambda_)
      real.append(fullobj)
      part.append(partobj)

    print('Real %f, part %f'%(np.mean(real), np.mean(part)))

