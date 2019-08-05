import argparse, pulp
import networkx as nx
import numpy as np
import random

def gen_graph(max_n, min_n, g_type='barabasi_albert'):
  cur_n = np.random.randint(max_n - min_n + 1) + min_n
  if g_type == 'erdos_renyi':
      g = nx.erdos_renyi_graph(n = cur_n, p = 0.15)
  elif g_type == 'powerlaw':
      g = nx.powerlaw_cluster_graph(n = cur_n, m = 4, p = 0.05)
  elif g_type == 'barabasi_albert':
      g = nx.barabasi_albert_graph(n = cur_n, m = 4)

  for edge in nx.edges(g):
    g[edge[0]][edge[1]]['weight'] = random.uniform(0,1)

  return g

def getEdgeVar(v1, v2, vert):
  u1 = min(v1, v2)
  u2 = max(v1, v2)
  if not ((u1, u2) in vert):
    vert[(u1, u2)] = pulp.LpVariable('u%d_%d'%(u1, u2), 0, 1, pulp.LpInteger)

  return vert[(u1, u2)]

def createOpt(G):
  prob = pulp.LpProblem('MILP Maximum Cut', pulp.LpMinimize)
  edgeVar = {}
  for j, (v1, v2) in enumerate(G.edges()):
    e12 = getEdgeVar(v1, v2, edgeVar)
    for u in G._adj[v1]:
      neigh = G._adj[u]
      if v2 in neigh:
        e23 = getEdgeVar(v2, u, edgeVar)
        e13 = getEdgeVar(v1, u, edgeVar)

        prob.addConstraint(e12 <= e13 + e23)
        prob.addConstraint(e12 + e13 + e23 <= 2)

  obj = 0
  for (v1, v2) in G.edges():
    e12 = getEdgeVar(v1, v2, edgeVar)
    obj = obj + (G[v1][v2]['weight'])*e12

  prob.setObjective(-1*obj) # Note that this is LpMinimum
  return prob

def cmdLineParser():
  '''
  Command Line Parser.
  '''
  parser = argparse.ArgumentParser(description='Minimum Vertex Cover')
  parser.add_argument('-o', dest='outPrefix', type=str, action='store', \
                      default=None, help='Output Prefix')
  parser.add_argument('-g', dest='g_type', type=str, action='store', \
                      default='erdos_renyi', help='Graph type')
  parser.add_argument('-max_n', dest='max_n', type=int, action='store', \
                      default=700, help='max number of nodes')
  parser.add_argument('-min_n', dest='min_n', type=int, action='store', \
                      default=500, help='min number of nodes')
  return parser.parse_args()

def generateInstance(max_n, min_n, g_type, outPrefix=None):
  G = gen_graph(max_n, min_n, g_type)
  P = createOpt(G)

  if outPrefix != None:
    # Write out
    nx.write_gpickle(G, outPrefix.replace('lpfiles', 'gpickle') + '.gpickle')
    P.writeLP(outPrefix + '.lp')
 
def main():
  args = cmdLineParser()
  G = gen_graph(args.max_n, args.min_n, args.g_type)
  P = createOpt(G)

  if args.outPrefix != None:
    # Write out
    nx.write_gpickle(G, args.outPrefix + '.gpickle')
    P.writeLP(args.outPrefix + '.lp')
    
if __name__ == '__main__':
  main()

