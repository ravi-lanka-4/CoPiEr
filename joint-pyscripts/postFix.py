import argparse, glob
import os
from getLog import procLog

def proc(fullLog, postFixLog, threshold=10):
  '''
  Process solutions
  '''
  fullsol = fullLog.replace('.log', '.sol')
  postfixsol = postFixLog.replace('.log', '.sol')

  fullgap = procLog(fullLog, 'dualgap')
  postfixgap = procLog(postFixLog, 'dualgap')
  print('Checking %s and %s'%(os.path.basename(fullLog), os.path.basename(postFixLog)))

  if postfixgap == None:
    postfixgap = 10000
  if fullgap == None:
    fullgap = 10000

  print('Duality Gap -> Full: %f Pre Fix: %f'%(fullgap, postfixgap))

  if (postfixgap > threshold) and (fullgap > threshold):
    print('Deleting %s'%os.path.basename(postfixsol))
    print('Deleting %s'%os.path.basename(fullLog))
    os.system('rm %s %s'%(fullsol, postfixsol))
  elif (postfixgap < fullgap):
    os.system('cp %s %s'%(postfixsol, fullsol))

  print('Final Sol File: %s'%os.path.basename(fullsol))
  return

def main(inpFolder, threshold):
  for inpF in glob.glob(inpFolder + '*.full.log'):
    postFixLog = inpF.replace('.full', '')
    proc(inpF, postFixLog, threshold)

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--i', type=str, default=None,
                       help='Input Folder', dest='inpFolder')
  parser.add_argument('--t', type=float, default=100,
                       help='Threshold', dest='threshold')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  if args.inpFolder:
    main(args.inpFolder, args.threshold)

