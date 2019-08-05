import argparse
import glob, os

def readObj(solF):
  obj = 100000
  if os.path.isfile(solF):
    with open(solF, 'r') as F:
      line = F.readline()
    print(line)
    obj = float(line.strip('objective value: "'))

  return obj

 
def main(inpF):
  for inpfile in glob.glob(inpF + '*.full.sol'):
    prevSolF = inpfile.replace('.full', '')
    fullobj = readObj(inpfile) 
    prevobj = readObj(prevSolF)

    # Copy only if the solution is better
    if fullobj < prevobj:
      print('Full is better: %f %f'%(fullobj, prevobj))
      os.system('mv %s %s'%(inpfile, prevSolF))
    else:
      print('Full is worse: %f %f'%(fullobj, prevobj))
  
  # Now remove all the full sols
  os.system('rm ' + inpF + '/*.full.sol')

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--i', type=str, default=None,
                       help='Input Folder', dest='inpFolder')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  if args.inpFolder:
    main(args.inpFolder)

