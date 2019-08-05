import argparse, os
import networkx as nx
import parser
import glob, pickle

def main(inpF, outF):
  '''
  Extracts the adjacency matrix and writes into a csv
  '''
  with open(inpF, 'rb') as F:
      G = pickle.load(F)

  A = nx.adjacency_matrix(G)
  A = A.toarray().ravel()
  
  # Write to output
  print('Writing: %s'%outF)
  with open(outF, 'w') as F:
      F.write('%d\n'%len(A))
      for val in A:
          F.write('%d\n'%val)

  return

def cmdLineParser():
  '''
  Command Line Parser.
  '''
  parser = argparse.ArgumentParser(description='Minimum Vertex Cover')
  parser.add_argument('-o', dest='outprefix', type=str, action='store', \
                      default='feats', help='Output prefix')
  parser.add_argument('-i', dest='inpFolder', type=str, action='store', \
                      default='../../../data/mvc_test/', help='Input prefix')

  return parser.parse_args()

if __name__ == '__main__':
  args = cmdLineParser()
  for inpF in glob.glob('%s/gpickle/*'%args.inpFolder):
      outF = inpF.replace('/gpickle/', '/feats/')
      os.system('mkdir -p %s'%outF)
      for cF in glob.glob(inpF + '/*.gpickle'):
        outF = cF.replace('/gpickle/', '/feats/').replace('.gpickle', '.csv')
        main(cF, outF)

