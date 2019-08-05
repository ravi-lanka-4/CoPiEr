import argparse, glob, os
import csv
import numpy as np
import pickle as P

def createCenterF(inpFile, outF):
 with open(inpFile, 'r') as F:
   data = P.load(F)
   vertices = data['V']
 
 centers = np.mean(vertices, axis=1)
 centers = centers.reshape(1, -1)[0]
 print('Creating: %s'%outF)
 with open(outF, 'wb') as F:
   pipe = csv.writer(F, delimiter=',')
   pipe.writerow([len(centers), ])
   for xx in centers:
     pipe.writerow([xx, ])

 return

def main(inpF):
 for (root, cDir, cFolder) in os.walk(inpF + '/'):
   for cF in cFolder:
     if '.pkl' in cF:
        createCenterF(root + '/' + cF, root + '/' + cF.replace('.pkl', '.csv'))

def firstPassCommandLine():

 # Creating the parser for the input arguments
 parser = argparse.ArgumentParser(description='Generate data for spatial model')

 # Positional argument - Input XML file
 parser.add_argument('--i', type=str, default='/Users/subrahma/proj/maverick-setup/co-training/data/psulu_mini_a/feats/',
                      help='Input Folder', dest='inpF')

 # Parse input
 args = parser.parse_args()
 return args


if __name__ == '__main__':
 args = firstPassCommandLine()
 main(args.inpF)

