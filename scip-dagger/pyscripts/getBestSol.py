import os, argparse, re
import glob, sys

def main(inpFolder, outF):
    '''
    Gets a list of sol files and finds the one with the least objective values
    '''
    bestobj = sys.float_info.max
    bestSolF = None
    for solF in glob.glob(inpFolder + '.*'):
      # Convert exponent objective value to float representation
      with open(solF, 'r') as fp:
        try:
          lines = fp.readlines()
          obj = float(lines[0].strip('objective value:\n').strip(' '))
          print('Objective Value: %f\n'%obj)
          if obj < bestobj:
              bestobj = obj
              bestSolF = solF 
        except:
          print('Solution likely infeasible')
          pass

    #assert bestSolF != None, "Sol File in an unexpected state"
    if bestSolF != None:
      os.rename(bestSolF, outF)
      print('Best objective: %f'%bestobj)

    # Remove all others
    for solF in glob.glob(inpFolder + '.*'):
        os.remove(solF)

    return

def firstPassCommandLine():
    
    # Creating the parser for the input arguments
    parser = argparse.ArgumentParser(description='Path Planning based on MILP')

    # Positional argument - Input XML file
    parser.add_argument('-i', type=str, default=None,
                        help='Input Folder to process the files', dest='inpFolder')
    parser.add_argument('-o', type=str, default=None,
                        help='Output Folder', dest='outFile')

    # Parse input
    args = parser.parse_args()
    return args

if __name__ == '__main__':
    args = firstPassCommandLine()
    if args.inpFolder != None:
        main(args.inpFolder, args.outFile)
