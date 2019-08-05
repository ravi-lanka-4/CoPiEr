import networkx as N
import glob, sys
  
def reWrite(inpF):
  print(inpF)
  g = N.read_gpickle(inpF)
  N.write_gpickle(g, inpF, 2)

def parse(inpFolder):
  for cF in glob.glob(inpFolder + '/**/*.gpickle', recursive=True):
    reWrite(cF)
  return

if __name__ == '__main__':
  parse(sys.argv[1])
