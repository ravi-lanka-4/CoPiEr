from __future__ import division
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from createSpatialInstance import readSol, getWaypt, readEnvi
from sympy import Point, Line, Segment
import numpy as np
import argparse, os
import pulp as P
import gurobipy as G
import yaml as Y
import csv

def main(inpF, solF, paramF, enviF, outPrefix, margin=0.1):
  '''
  Augmenting the data 
  '''
  assert os.path.isfile(inpF), 'Input %s file missing'%inpF
  assert os.path.isfile(solF), 'Solution %s file missing'%solF
  assert os.path.isfile(paramF), 'Param %s file missing'%paramF
  assert os.path.isfile(enviF), 'Environment %s file missing'%enviF
  print('Augmenting file : %s'%inpF)

  nsides = 4
  os.system('mkdir -p %s'%os.path.basename(outPrefix))
  csol = readSol(inpF, solF)
  (obstacles, nObst) = readEnvi(enviF)
  (nSteps, allSteps, stepsToVar) = getWaypt(csol)

  # Read radius from configuration file
  with open(paramF) as stream:
    params = Y.load(stream)
    radius = params['max_velocity']

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

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--i', type=str, default='/Users/subrahma/proj/psulu/data/sample-input.lp',
                       help='Solution File', dest='inpF')
  parser.add_argument('--s', type=str, default='/Users/subrahma/proj/psulu/data/sample-solution.sol',
                       help='Solution File', dest='solF')
  parser.add_argument('--p', type=str, default='/Users/subrahma/proj/psulu/data/sample-param.yaml',
                       help='Param File', dest='paramF')
  parser.add_argument('--e', type=str, default='/Users/subrahma/proj/psulu/data/sample-envi.yaml',
                       help='Environment File', dest='enviF')
  parser.add_argument('--o', type=str, default='/Users/subrahma/proj/psulu/data/',
                       help='Output Folder', dest='outPrefix')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  main(args.inpF, args.solF, args.paramF, args.enviF, args.outPrefix)

