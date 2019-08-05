from __future__ import division
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np
import argparse, os
import pulp as P
#import gurobipy as G
import yaml as Y
import csv, random, math
from sympy import Point, Line, Segment
import PIL
from PIL import Image
from shapely.geometry.polygon import Polygon
from shapely.geometry import Point as P
from scipy.spatial.distance import euclidean
from spatialModel import spatialModel

# Should be moved to a single config file setup
img_rows, img_cols = 128, 96

def getRand(minval=0, maxval=1):
    return random.uniform(minval, maxval)

def readEnvi(enviF):
  # Read environment
  with open(enviF, 'r') as stream:
    # Stream of obstacles
    envParams = Y.load(stream)

    # Initialize the class objects
    obstacles = envParams['environment']['obstacles']
    nObst = len(obstacles.keys())

  return (obstacles, nObst)

def readSol(inpF, solF):
  # Read file and sol file
  csol = G.read(inpF)
  csol.update()
  csol.read(solF)
  csol.setParam("NodeLimit", 1)
  csol.setParam("DisplayInterval", 1)
  csol.optimize()
  return csol

def _readSol(solF):
  with open(solF, 'r') as F:
    cdata = F.readlines()

  # Extract only the position data
  xdata = []
  for cline in cdata:
    if 'x_' in cline:
      xdata.append(cline)
  assert len(xdata) > 0, "No data for state variables in the solution"

  # Convert to steps
  nSteps = int(len(xdata)/4)
  cdata = np.zeros((nSteps, 2))
  for x in xdata:
    try:
      stepstr, val = x.split(' ')
    except:
      cx = x.split(' ')
      stepstr = cx[0]
      val = cx[-2]

    val = val.strip('\n')
    (step, dim) = stepstr.replace('x_', '').split('_')
    step = int(step)
    dim = int(dim)
    if dim < 2:
      cdata[step, dim] = float(val)

  return (nSteps, cdata)

def getWaypt(csol):
  # Read number of steps
  cnt = 0
  stepsToVar = {}
  for v in csol.getVars():
    if 'x_' in v.varName:
      cnt += 1
      name = v.varName
      (step, side) = name.replace('x_', '').split('_')
      stepsToVar[(int(step), int(side))] = v.X

  nSteps = int(len(stepsToVar)/4)
  allSteps = np.zeros((int(nSteps), 4))
  for k, v in stepsToVar.iteritems():
      allSteps[k[0], k[1]] = v
  allSteps = allSteps[:, :2]

  return (nSteps, allSteps, stepsToVar)

def generateObst(numObst, idxoffset=0, minW=0.08, maxW=0.08, minL=0.08, maxL=0.08):
  # Creating the obstacles
  obst = {}
  for n in range(numObst):
     cObst = {}
     cObst['shape'] = 'polygon'

     # Get random Length and width
     width  = getRand(minW, maxW)
     length = getRand(minL, maxL)
     rectTemplate = np.array([[0,0], [width, 0], [width, length], [0, length]])

     # Get a random rotation
     angle = np.pi*getRand()
     rotationMat = np.array([[math.cos(angle), -math.sin(angle)], \
      		       [math.sin(angle),  math.cos(angle)]]).T
     offset = np.array([width/2, length/2])
     rotatedRect = np.dot(rectTemplate - offset, rotationMat) + offset

     # Get random displacement
     dispW = getRand(0, 1)
     dispL = getRand(0, 1)
     rotatedRect = rotatedRect + np.array([dispW, dispL])

     # Compile the corners into the dictionary
     cObst['corners'] = rotatedRect.tolist()
     obstName = 'obs_%d'%(n+idxoffset)
     obst[obstName] = cObst

  return obst

def pltResult(enviF, pred, target, outF):

  # Read environment
  with open(enviF, 'r') as stream:
    # Stream of obstacles
    envParams = Y.load(stream)

    # Initialize the class objects
    obstacles = envParams['environment']['obstacles']
    nObst = len(obstacles.keys())

  ## Plot obstacles
  for obst in obstacles.keys():
    corners = np.array(obstacles[obst]['corners'])
    x = corners[:,0]
    y = corners[:,1]
    plt.fill(x, y, color='black')

  for points in [pred, target]:
    if not points:
      continue

    for i, (pt1, pt2) in enumerate(zip(points[:-1], points[1:])):
      plt.plot([pt1[0], pt2[0]], [pt1[1], pt2[1]], c='r')
      plt.scatter(pt1[0], pt1[1], c='g')
      plt.scatter(pt2[0], pt2[1], c='g')

  plt.savefig(outF)
  plt.close('all')
  return

def createBoundary(allSteps, margin):
  marginStep1 = []
  marginStep2 = []

  # Border conditions
  first = Point(-allSteps[1][0], -allSteps[1][1])
  last = Point(-(allSteps[-2][0]-1)+1, -(allSteps[-2][1]-1)+1)

  for i in range(0, len(allSteps)):
    origin = Point(0, 0)
    xLine = Line(Point(0, 1), origin)

    # Decide the three set of poitns to work
    pt1 = Point(allSteps[i][0], allSteps[i][1])

    if i == len(allSteps)-1: 
      pt2 = last
    else:
      pt2 = Point(allSteps[i+1][0], allSteps[i+1][1])

    if i == 0:
      pt3 = first
    else:
      pt3 = Point(allSteps[i-1][0], allSteps[i-1][1])

    # Translate to origin
    vec1 = pt2 - pt1
    vec2 = pt3 - pt1
    seg1 = Segment(pt2 - pt1, origin)
    seg2 = Segment(pt3 - pt1, origin)

    # Get the bisecting line
    scale = float(seg1.length/seg2.length)
    newpt3 = pt3-pt1
    newpt3 = Point(scale*newpt3[0], scale*newpt3[1])

    # Get perpendicular bisection
    seg = Segment(pt2 - pt1, newpt3)
    perpLine = seg.perpendicular_bisector()
    
    # Rescale
    newpt4 = perpLine.parallel_line(origin).points[1]
    newSeg = Segment(newpt4, origin)
    scale = margin/float(newSeg.length)
    newpt4 = Point(scale*newpt4[0], scale*newpt4[1])
    newpt5 = Point(-newpt4[0], -newpt4[1])

    realpt4 = [newpt4[0] + pt1[0], newpt4[1] + pt1[1]]
    realpt5 = [newpt5[0] + pt1[0], newpt5[1] + pt1[1]]

    if not marginStep1:
        marginStep1.append(realpt4)
        marginStep2.append(realpt5)
    else:
        dist1 = float(Segment(marginStep1[-1], realpt4).length)
        dist2 = float(Segment(marginStep2[-1], realpt4).length)
        if dist1 < dist2:
           marginStep1.append(realpt4)
           marginStep2.append(realpt5)
        else:
           marginStep2.append(realpt4)
           marginStep1.append(realpt5)

  return (marginStep1, marginStep2)

def checkOverlap(obst, allSteps, margin, poly):
  obstname = obst.keys()[0]
  flag = False
  for corner in obst[obstname]['corners']:
    cornerPt = P(corner[0], corner[1])
    flag = flag or poly.contains(cornerPt)

  if flag:
    return False
  else:
    return True

def createNewObst(allSteps, numObst, offset=0, margin=0.15):
  '''
  Create a new set of obstacles
  Offset in order to not conflict with the name of the obstacle
  '''
  count = 0
  allObst = {}

  # create polytope around path
  margin1, margin2 = createBoundary(allSteps, margin)
  margin1 = np.array(margin1).astype(float)
  margin2 = np.array(margin2).astype(float)
  margin2 = margin2[::-1]
  marginpts = np.vstack(([0,0], margin1, [1,1], margin2))
  poly = Polygon(marginpts)
  
  while count < numObst:
    obst = generateObst(1, offset + count)
    noOverlap = checkOverlap(obst, allSteps, margin, poly)
    if noOverlap:
        print('Added new obst %d'%count)
        allObst.update(obst)
        count = count + 1

  return allObst

def prepData(inpF, stateVec, step):
  img = Image.open(inpF)
  img = img.convert('RGB')
  dim = [img_rows, img_cols]
  img = img.resize(dim[::-1], PIL.Image.ANTIALIAS)
  img = np.array(img).astype(float)
  img = img/255
  inp1 = np.array([[stateVec[0], stateVec[1]]])

  # Prepare input
  M = {}
  M['input_2'] = img.reshape((1,) + img.shape)
  M['input_1'] = inp1

  return M

def _write(outF, data, mode='a'):
  with open(outF, "a") as csvF:
    writer = csv.writer(csvF)
    for cline in data:
      writer.writerow(cline)
  return

def _writesol(outF, data, mode='w'):
  '''
  Writes partial solution
  '''
  with open(outF, 'w') as solF:
    for i, step in enumerate(data):
      for j, val in enumerate(step):
        str_ = 'x_%d_%d %f\n'%(i, j, val)
        solF.write(str_)
      for j in range(2, 4):
        str_ = 'x_%d_%d 0\n'%(i, j)
        solF.write(str_)
  return

def filter_waypoints(wp):
  '''
  Removes way points too close to each other for 
  numerical stability
  '''
  if len(wp) < 2:
      return wp

  newWp = []
  wp = np.array(wp)
  for prev, curr in zip(wp[:-1], wp[1:]):
    dist_ = euclidean(prev, curr)
    if dist_ > (10**-3):
        newWp.append(prev.tolist())
  newWp.append(wp[-1, :].tolist())

  if len(newWp) == len(wp):
    return newWp
  else:
    return filter_waypoints(newWp)


def main(inpF, solF, paramF, enviF, outPrefix, model=None, augment=False, numObst=1, suffix=''):
  '''
  Read input lp file and corresponding solution
  '''
  assert os.path.isfile(inpF), 'Input %s file missing'%inpF
  assert os.path.isfile(solF) or (model != None), 'Solution %s file missing'%solF
  assert os.path.isfile(paramF), 'Param %s file missing'%paramF
  assert os.path.isfile(enviF), 'Environment %s file missing'%enviF
  print('Processing: %s'%inpF)

  if model != None:
    augment = False

  nsides = 4
  (obstacles, nObst) = readEnvi(enviF)

  # Read radius from configuration file
  with open(paramF) as stream:
    params = Y.load(stream)
    radius = params['max_velocity']

  if model==None:
    if False:
      csol = readSol(inpF, solF)
      (nSteps, allSteps, stepsToVar) = getWaypt(csol)
    else:
      nSteps, allSteps = _readSol(solF)

    # Remove way points too close to each other
    allSteps = filter_waypoints(allSteps)
    nSteps = len(allSteps)
    isSol = True
  else:
    nSteps = params['waypoints'] + 1
    allSteps = np.zeros((nSteps, 2))
    isSol = False

  if augment:
    newObst = createNewObst(allSteps, numObst, nObst)
    obstacles.update(newObst)
    nObst = len(obstacles)

  # Create each data point of spatial model training
  target = []
  stateVec = np.zeros((nsides, ))
  predSteps = [[0, 0]]
  trueSteps = []
  for j, cstep in enumerate(range(nSteps-1)):
    # Collect steps
    if not model:
      stateVec = allSteps[cstep]
    else:
      trueSteps.append(stateVec)

    ## Plot obstacles
    fig1 = plt.figure(frameon=False)
    ax1 = fig1.add_subplot(111, aspect='equal')
    boundary = patches.Circle(stateVec[:3], radius, color='g')
    #center = patches.Circle(stateVec[:2], 0.01, color='g')
    ax1.add_patch(boundary)
    #ax1.add_patch(center)
    ax1.axis('off')
    plt.axis([-0.2, 1.2, -0.2, 1.2])

    cornerpt = []
    for obst in obstacles.keys():
      corners = np.array(obstacles[obst]['corners'])
      x = corners[:,0]
      y = corners[:,1]
      plt.fill(x, y, color='black')
      cornerpt.extend(x.tolist())
      cornerpt.extend(y.tolist())

    nextStateVec = np.array(allSteps[cstep+1]) - np.array(stateVec[:2])

    if augment:
      invert_x = np.random.rand() > 0.5
      invert_y = np.random.rand() > 0.5
      if invert_x:
          cstateVecX = 2-stateVec[0]
          cnextStateVecX = -nextStateVec[0]
      else:
          cstateVecX = stateVec[0]
          cnextStateVecX = nextStateVec[0]

      if invert_y:
          cstateVecY = 2-stateVec[1]
          cnextStateVecY = -nextStateVec[1]
      else:
          cstateVecY = stateVec[1]
          cnextStateVecY = nextStateVec[1]

    #plt.show()
    outF = outPrefix + '_%d%s.png'%(j, suffix)
    plt.tight_layout()

    # Invert the plot
    if augment: 
        if invert_x:
            plt.gca().invert_xaxis()
        if invert_y:
            plt.gca().invert_yaxis()

    print('Creating: '+outF)
    plt.savefig(outF, bbox_inches='tight', pad_inches=0)
    if model == None:
      if augment:
          out = [outF, '%f'%cstateVecX, '%f'%cstateVecY, \
                                  '%f'%cnextStateVecX, '%f'%cnextStateVecY]
      else:
          out = [outF, '%f'%stateVec[0], '%f'%stateVec[1], \
                                  '%f'%nextStateVec[0], '%f'%nextStateVec[1]]
      #out.extend(cornerpt)
      target.append(out)
    else:
      inp = prepData(outF, stateVec, j/nSteps)
      out = model.predict(inp)[0]
      stateVec[0] = stateVec[0] + out[0]
      stateVec[1] = stateVec[1] + out[1]
      predSteps.append([stateVec[0], stateVec[1]])

      # Remove the temporary file
      os.system('rm %s'%outF)
    plt.clf()
    plt.close('all')

  # Write target for the input
  if model == None:
    targetF = os.path.dirname(outPrefix) + '/target.txt'
    _write(targetF, target)
  else:
    # Write predicted output
    outF = outPrefix.replace('.lp', '') + '.part.sol'
    _writesol(outF, predSteps, 'w')

    # Write out plot
    pltF = outPrefix.replace('.lp', '') + '.png'
    if isSol:
      trueSteps.append(allSteps[nSteps-1])
    else:
      trueSteps = None
    pltResult(enviF, predSteps, trueSteps, pltF)

  return

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--i', type=str, default='/Users/subrahma/proj/pdf/co-training/data//psulu_mini//lpfiles/unlabeled//input452.lp',
                       help='Solution File', dest='inpF')
  parser.add_argument('--s', type=str, default='/Users/subrahma/proj/pdf/co-training/data//psulu_mini//sol/unlabeled//input452.sol',
                       help='Solution File', dest='solF')
  parser.add_argument('--p', type=str, default='/Users/subrahma/proj/pdf/co-training/joint-pyscripts/data-generator/psulu/config/param.yaml',
                       help='Param File', dest='paramF')
  parser.add_argument('--e', type=str, default='/Users/subrahma/proj/pdf/co-training/data//psulu_mini//env/unlabeled//envi452.yaml',
                       help='Environment File', dest='enviF')
  parser.add_argument('--o', type=str, default='/Users/subrahma/proj/psulu/data/testing',
                       help='Output Folder', dest='outPrefix')
  parser.add_argument('--m', type=bool, default=False,
                       help='Use Model', dest='model')
  parser.add_argument('--f', type=str, default=None,
                       help='Model file', dest='modelF')
  parser.add_argument('--a', type=bool, default=False,
                       help='Augment data', dest='augment')
  parser.add_argument('--n', type=int, default=3,
                       help='Number of new obstacles', dest='numObst')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  if args.model:
      model = spatialModel(img_rows, img_cols)
      if '.h5' in args.modelF:
        model.load_weights(args.modelF)
      else:
        model.load_weights(args.modelF + '.h5')
  else:
      model = None

  main(args.inpF, args.solF, args.paramF, args.enviF, args.outPrefix, model, args.augment, args.numObst)

