import yaml as Y
import argparse
import pdb
import os, sys
import numpy as np
import random as R

from numpy import linalg as LA
from scipy.spatial.distance import euclidean

#Tiago: should this class be called ObstacleMap. Obstacle sounds like one object only.
#       Use explicit name of variables as oppose to H, V, g etc. it make it hard to read ;)
class ObstacleMap():
  '''
  Maintains the obstacle Map
  Currently doesn't support
  '''
  def __init__(self, obstMapFile=None):
    self.nObstacles     = None
    self.nSides         = None

    # Standard names used in the paper
    # H -> Polytope boundaries, V -> Corner Vertices, g->Plane Constant
    self.obstNormal   = None #H
    self.obstVert     = None #V
    self.obstOffset   = None #G
    self.obstName     = None

    # Deleted obstacles
    self.delVert = None

    if obstMapFile is not None:
      # Initializing the nSides to 4 for now - To be generalized
      self.nSides = 4

      self.__readObstMap__(obstMapFile)
      (self.obstNormal, self.obstOffset) = self.__computeHG__()

  def delObst(self, idx):
    self.nObstacles = self.nObstacles - len(idx)
    self.obstNormal = np.delete(self.obstNormal, idx, 0)
    self.obstVert   = np.delete(self.obstVert, idx, 0)
    self.obstOffset = np.delete(self.obstOffset, idx, 0)
    self.obstName   = np.delete(self.obstName, idx, 0)
    return

  def __readObstMap__(self, obstMapFile):
    '''
    Read Obstacle map from the YAML
    '''

    # Read Obstacle YAML map
    print('Reading Obstacle Map: %s'%obstMapFile)
    stream = file(obstMapFile, 'r')
    envParams = Y.load(stream)

    # Initialize the class objects
    obstacles = envParams['environment']['obstacles']
    self.nObstacles = len(obstacles.keys())

    # Read obstancles from the environment file
    self.obstVert = np.zeros((self.nSides,2,self.nObstacles))
    self.obstName = []
    self.zInit    = []
    for i, obstName in enumerate(obstacles.keys()):
      self.obstVert[:,:,i] = obstacles[obstName]['corners']
      self.obstName.append(obstName)
      try:
         # Search for initialization variables
         cKeys = obstacles[obstName].keys()
         for k in cKeys:
           if 'init' in k:
              stepnum = int(k.replace('init_',''))
              self.zInit.append({'stepnum': stepnum, 'obstname': obstName, 'obstnum': i,\
					'side': obstacles[obstName][k]})
      except:
         continue

    return

  def __computeHG__(self):
    '''
    Computes H and G from the corners
    '''
    # Initialize
    obstVert  = np.array(self.obstVert)
    nObst     = self.nObstacles
    nSides    = self.nSides

    # Lets compute sides
    obstNormal = np.zeros((self.nSides,2,self.nObstacles))
    obstOffset = np.zeros((nObst,nSides))
    for obst in range(nObst):
        for side in range(nSides-1):
          obstNormal[side,:,obst] = [obstVert[side+1,1,obst]-obstVert[side,1,obst], \
					obstVert[side,0,obst]-obstVert[side+1,0,obst]]
          obstNormal[side,:,obst] = obstNormal[side,:,obst]/\
					LA.norm(obstVert[side+1,:,obst]-obstVert[side,:,obst],2)
          obstOffset[obst,side]   = np.dot(obstNormal[side,:,obst], obstVert[side,:,obst])

        obstNormal[nSides-1,:,obst] = [obstVert[0,1,obst]-obstVert[nSides-1,1,obst], \
					obstVert[nSides-1,0,obst]-obstVert[0,0,obst]];
        obstNormal[nSides-1,:,obst] = obstNormal[nSides-1,:,obst]/\
					LA.norm(obstVert[0,:,obst]-obstVert[nSides-1,:,obst],2)
        obstOffset[obst,nSides-1]   = np.dot(obstNormal[nSides-1,:,obst], \
						obstVert[nSides-1,:,obst])

    obstNormal = np.rollaxis(obstNormal, 2)
    self.obstVert = np.rollaxis(obstVert, 2)
    return (obstNormal, obstOffset)

class pSulu(object):
  '''
  Base class for different pSulu Implementations
  '''

  def __init__(self, configFile=None):
    self.environment             = None
    self.outFolder               = None
    self.start_location          = None
    self.end_location            = None
    self.chance_constraint       = None
    self.waypoints               = None
    self.coVarX                  = None
    self.coVarY                  = None
    self.cleanMap                = None
    self.cleanDist               = None
    self.receding_horizon        = None
    self.horizon_cost_proportion = None
    self.horizon_polygon_size    = None
    self.margin                  = None
    self.budget                  = None
    self.quad                    = None

    if configFile is not None:
      self.__parseConfig__(configFile)
      self.max_velocity = self.budget*float(np.sqrt(2))/self.waypoints
      self.max_control  = 0 #self.budget*float(np.sqrt(2))/self.waypoints
      self.obstMap = self.readEnvironment()

      # Fixing Covariance
      self.coVarY = self.coVarY or 0.001
      self.coVarX = self.coVarX or 0.001

  def clean(self, threshold=None):
    '''
    Removes obstacles that are really close to the goal and destination
    '''
    if threshold == None:
       threshold = self.cleanDist
      
    delList = []
    for j, obst in enumerate(self.obstMap.obstVert):
      flag = False
      for v in obst:
        startdist = euclidean(v, self.start_location[:2])
        goaldist = euclidean(v, self.end_location[:2])
        if (startdist < self.cleanDist) or (goaldist < threshold):
          flag = True
          print('Deleting %s'%self.obstMap.obstName[j])
          break

      if flag:
        # delete the obstacle because its too close to the goal
        delList.append(j)
        
    # Delete the corresponding obstacles
    self.obstMap.delObst(delList)
    return
        
  def __parseConfig__(self, configFile):
    '''
    Reads the config file and initialises the objects
    '''

    # Read the config file
    stream = open(configFile, 'r')
    configParams = Y.load(open(configFile, 'r'))

    # Initialize the class objects
    for params in vars(self).keys():
      try:
        setattr(self, params, configParams[params])
      except:
	print('Attribute %s missing in the parameters'%params)
        if params == "start_location":
            self.start_location = [0, R.random(), 0, 0]
        elif params == "end_location":
            self.end_location = [1, R.random(), 0, 0]

    # Convert string to native types
    self.baseFolder = os.path.dirname(os.path.abspath(configFile)) + '/'
    self.__convertType__()
    return

  def __convertType__(self):
    '''
    Converts string to native datatype - Hardcoded for now
    To be improvised later
    '''
    self.environment = self.environment.strip('[()]')
    self.outFolder   = self.outFolder.strip('[()]')
    self.start_location = [float(xx) for xx in self.start_location]
    self.end_location = [float(xx) for xx in self.end_location]
    self.cleanMap = bool(self.cleanMap)
    
  def readEnvironment(self):
    '''
    Reads the obstacle map
    '''
    obstMap = ObstacleMap(self.environment)
    return obstMap

def main(args):
  inputConfig = args.inputConfig

  # Creating instance
  psulu = pSulu(inputConfig)
  return

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Path Planning based on MILP')

  # Positional argument - Input XML file
  parser.add_argument('-input', '--i', type=str, default='./config/input.yaml',
                      help='Input Configuration File', dest='inputConfig')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  main(args)
