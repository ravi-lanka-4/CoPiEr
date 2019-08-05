import ctypes
import networkx as nx
import numpy as np
import os
import sys

class MaxcutLib(object):

    def __init__(self, args):
        dir_path = os.path.dirname(os.path.realpath(__file__))        
        self.lib = ctypes.CDLL('%s/build/dll/libmaxcut.so' % dir_path)

        self.lib.Fit.restype = ctypes.c_double
        self.lib.Test.restype = ctypes.c_double
        self.lib.TestNoStop.restype = ctypes.c_double
        self.lib.GetSol.restype = ctypes.c_double
        arr = (ctypes.c_char_p * len(args))()
        arr[:] = args
        self.lib.Init(len(args), arr)
        self.ngraph_train = 0
        self.ngraph_test = 0

    def __CtypeNetworkX(self, g):
        edges = g.edges(data='weight') #list(g.edges_iter(data='weight', default=1))
        e_list_from = (ctypes.c_int * len(edges))()
        e_list_to = (ctypes.c_int * len(edges))()
        weights = (ctypes.c_double * len(edges))()

        if len(edges):
            a, b, c = zip(*edges)
            e_list_from[:] = a
            e_list_to[:] = b
            weights[:] = c

        return (len(g.nodes()), len(edges), ctypes.cast(e_list_from, ctypes.c_void_p), \
                ctypes.cast(e_list_to, ctypes.c_void_p), ctypes.cast(weights, ctypes.c_void_p))

    def TakeSnapshot(self):
        self.lib.UpdateSnapshot()

    def ClearTrainGraphs(self):
        self.ngraph_train = 0
        self.lib.ClearTrainGraphs()

    def InsertGraph(self, g, is_test, opt=None, prob=0.5):
        n_nodes, n_edges, e_froms, e_tos, weights = self.__CtypeNetworkX(g)

        opt_sol = False
        if opt != None:
          opt_list_v1 = (ctypes.c_int * len(opt))()
          opt_list_v2 = (ctypes.c_int * len(opt))()
          opt_list_v1[:] = [x[0] for x in opt]
          opt_list_v2[:] = [x[1] for x in opt]
          opt_list_v1 = ctypes.cast(opt_list_v1, ctypes.c_void_p)
          opt_list_v2 = ctypes.cast(opt_list_v2, ctypes.c_void_p)
          opt_sol = True

        if is_test:
            t = self.ngraph_test
            self.ngraph_test += 1
        else:
            t = self.ngraph_train
            self.ngraph_train += 1

        if opt_sol:
            self.lib.InsertGraph.argtypes = [ctypes.c_bool, ctypes.c_int, ctypes.c_int, ctypes.c_int, \
                                              ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, 
                                              ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p, \
                                              ctypes.c_int, ctypes.c_double]
            self.lib.InsertGraph(is_test, t, n_nodes, n_edges, e_froms, e_tos, weights, \
                                    len(opt), opt_list_v1, opt_list_v2, len(opt), prob)
        else:
            self.lib.InsertGraph.argtypes = [ctypes.c_bool, ctypes.c_int, ctypes.c_int, ctypes.c_int, \
                                              ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int]
            self.lib.InsertGraph(is_test, t, n_nodes, n_edges, e_froms, e_tos, weights, 0)

    
    def LoadModel(self, path_to_model):
        p = ctypes.cast(path_to_model, ctypes.c_char_p)
        self.lib.LoadModel(p)

    def SaveModel(self, path_to_model):
        p = ctypes.cast(path_to_model, ctypes.c_char_p)
        self.lib.SaveModel(p)

    def GetSol(self, gid, maxn):
        sol = (ctypes.c_int * (maxn + 10))()
        val = self.lib.GetSol(gid, sol)
        return val, sol

if __name__ == '__main__':
    f = MaxcutLib(sys.argv)
