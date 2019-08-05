from __future__ import division
import argparse
import yaml
import random
import numpy as np
import math

def firstPassCommandLine():
    
    # Creating the parser for the input arguments
    parser = argparse.ArgumentParser(description='Path Planning based on MILP')

    # Positional argument - Input XML file
    parser.add_argument('-n', type=int, default=10,
                        help='Number of obstacles', dest='numObst')
    parser.add_argument('-o', type=str, default='./config/newEnvi.yaml',
                        help='Output File name', dest='outFile')
    parser.add_argument('-maxL', type=int, default=0.075,
                        help='Max Length', dest='maxLen')
    parser.add_argument('-minL', type=int, default=0.075,
                        help='Min Length', dest='minLen')
    parser.add_argument('-maxW', type=int, default=0.075,
                        help='Max Width', dest='maxWid')
    parser.add_argument('-minW', type=int, default=0.075,
                        help='Min Width', dest='minWid')

    # Parse input
    args = parser.parse_args()
    return args

def getRand(minval=0, maxval=1):
    '''
    Generates a random number with uniform distribution in a given range
    '''	
    return random.uniform(minval, maxval)

def main(args):
    # Parameters
    numObst = args.numObst
    minL    = args.minLen 
    maxL    = args.maxLen
    minW    = args.minWid
    maxW    = args.maxWid
    outF    = args.outFile
  
    # Creating the obstacles
    obst = {}
    obst['environment'] = {}
    obst['environment']['obstacles'] = {}
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
       obstName = 'obs_%d'%(n)
       obst['environment']['obstacles'][obstName] = cObst

    with open(outF, 'w') as outfile:
       yaml.dump(obst, outfile, explicit_start=True)

if __name__ == '__main__':
    args = firstPassCommandLine()
    main(args)

