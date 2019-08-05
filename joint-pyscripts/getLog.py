import argparse, glob

def procLog(inpF, tag='dualgap'):
  '''
  Process log file
  '''
  with open(inpF, 'r') as F:
    data = F.readlines()

  if 'dualgap' in tag:
    dualgap = None
    for cdata in data:
      if dualgap:
        break

      if 'Gap' in cdata:
        cc = cdata.strip('Gap: ')
        cc = cc.replace('%', '') 
        try:
          dualgap = float(cc)
        except:
          pass

    return dualgap

  elif 'nodes' in tag:
    nnodes = None
    for cdata in data:
      if nnodes:
        break

      if 'Solving Nodes' in cdata:
        cc = cdata.replace('Solving Nodes', '')
        cc = cc.strip(': ')
        try:
          nnodes = int(cc)
        except:
          pass

    return nnodes

  elif 'time' in tag:
    runTime = None
    for cdata in data:
      if runTime:
        break

      if 'Total Time' in cdata:
        cc = cdata.replace('Total Time', '')
        cc = cc.strip(': ')
        try:
          runTime = float(cc)
        except:
          pass

    return runTime

  return

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--i', type=str, default='tmp.log',
                       help='Log file', dest='inpF')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  if args.inpF:
    for tag in ['dualgap', 'nodes', 'time']:
      val = procLog(args.inpF, tag)
      print('%s: %f'%(tag, val))


