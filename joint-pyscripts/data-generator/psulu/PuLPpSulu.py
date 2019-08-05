#!/usr/local/python
from __future__ import division
import socket, pickle
import matplotlib as M

# Set the display on server 
hostname = socket.gethostname().split('.')[0]
if ('crunchy' in hostname) or ('cuda' in hostname):
  M.use('Agg')

import matplotlib.pyplot as plt
import matplotlib.patches as patches

from scipy import ndimage, misc, stats
from pSulu_api import pSulu

import re
import tempfile
import math, os
import mpmath as mp
import numpy as np
from termcolor import colored

# Gurobi related environment variable paths
os.environ["PATH"] += ':/usr/local/bin/'
import argparse
import sys, pdb, math
import yaml as Y
import pulp
from pyscipopt import Model

usePuLP=False

def createDict(s, dim1, dim2, type_="CONTINUOUS", lb=None, ub=None, str_=None):
  var = {}
  for i in range(dim1):
    var[i] = {}
    for j in range(dim2):
      var[i][j] = s.addVar(name='%s_%d_%d'%(str_, i, j), vtype=type_, lb=lb, ub=ub)
  return var

def create1Dict(s, dim1, type_="CONTINUOUS", lb=None, ub=None, str_=None):
  var = {}
  for i in range(dim1):
    var[i] = s.addVar(name='%s_%d'%(str_, i), vtype=type_, lb=lb, ub=ub)
  return var

class PathSolver:
    def __init__(self, u_max, xInit, xT, A, B, horizon, margin=None, poly_nsides=50, horizon_cost=0.5):
        '''
        Mixed integer programming based path planning
        '''
        # Initialization parameters
        self.xInit = xInit
        self.xT    = xT

        # create solver instance
        if usePuLP:
           self.prob = pulp.LpProblem("MILP Path Solver", pulp.LpMinimize)
           # Remove noOverlap to update the constraints on recursive calls
           self.prob.noOverlap = 0
        else:
           self.prob = Model('Quadratic Path Solver')
           self.cnstMap = {}

        # Result x and y positions
        self.wayPoint           = [] 
        self.activePoints       = []
        self.activePtsIdx       = []
        self.activeObstIdx      = []
        self.initNames          = []
        self.nNodes             = None

        # Receding horizon related parameter
        self.poly_nsides        = poly_nsides
        self.receding_horizon   = horizon
        self.horizon_cost       = horizon_cost
        if self.receding_horizon:
          # Check for the range of horizon cost
          # ranges from [0,1] for computing a convex combination
          if ((self.horizon_cost < 0) or (self.horizon_cost > 1)):
            raise Exception('Horizon cost should be between 0 and 1 (Convex combination parameter)')
        else:
          self.horizon_cost = 0

        # Local hidden variables
        self.__N        = None
        self.__u_max    = u_max
        self.u_max      = u_max
        self.__A        = A
        self.__B        = B
        self.gap        = None
        self.margin     = margin
        if self.margin != None:
            if self.margin < 0:
                raise Exception('Margin should be positive')

        self.msg        = 1
        self.__nu       = len(self.__B[0])
        self.__nx       = len(self.__A[0])
        self.__M        = 10000 
        self.__epsilon  = 0.0001
        self.__nObst    = None
        self.__dObst    = None
        self.__obstIdx  = {}
        self.__active   = None
        self.__zVal     = []
        self.__H        = []
        self.__V        = None
        self.__G        = []
        self.__sol      = []

        return
  
    def __createVar(self):
        # Real variables 
        if usePuLP:
          self.u = pulp.LpVariable.dicts("u", (range(self.__N), range(self.__nu)), \
                                                  -self.__u_max, self.__u_max)
          self.absu = pulp.LpVariable.dicts("absu", (range(self.__N), range(self.__nu)), \
                                                  self.__epsilon, self.__u_max)
          self.x = pulp.LpVariable.dicts("x", (range(self.__N+1), range(self.__nx)))

          if self.receding_horizon:
            # Define the distance between the last time step and goal
            self.dlast = pulp.LpVariable("dlast", lowBound=0)
            self.d = pulp.LpVariable.dicts("d", range(self.__N-1), lowBound=0)
          else:
            self.d = pulp.LpVariable.dicts("d", range(self.__N), lowBound=0)
            self.dlast = None
        else:
          self.u = createDict(self.prob, self.__N, self.__nu, type_="CONTINUOUS", \
                                lb=-1*self.__u_max, ub=self.__u_max, str_="u")
          self.absu = createDict(self.prob, self.__N, self.__nu, type_="CONTINUOUS", \
                                lb=self.__epsilon, ub=self.__u_max, str_="absu")
          self.x = createDict(self.prob, self.__N+1, self.__nx, type_="CONTINUOUS", str_="x")

          if self.receding_horizon:
            # Define the distance between the last time step and goal
            self.dlast = self.prob.addVar(name="dlast", lb=0)
            self.d = create1Dict(self.prob, self.__N-1, str_="d", lb=0)
          else:
            self.d = create1Dict(self.prob, self.__N, str_="d", lb=0)
            self.dlast = None

        return

    def __addObjective(self):
        # Problem Objective
        # self.d == 0 when recending horizon is off
        if usePuLP:
          if not self.receding_horizon:
            self.prob += pulp.lpSum([self.d[i] for i in range(self.__N)])
          else:
            self.prob += (1-self.horizon_cost)*pulp.lpSum([self.d[i] for i in range(self.__N-1)]) + self.horizon_cost*self.dlast
        else:
          if not self.receding_horizon:
            obj = 0
            for i in range(self.__N):
              obj += self.d[i]
            self.prob.setObjective(obj)
          else:
            obj = 0
            for i in range(self.__N):
              obj += self.d[i]
            self.prob.setObjective((1-self.horizon_cost)*obj + self.horizon_cost*self.dlast)

        return

    def addAllZConstraints(self):
        for k in range(self.__N): 
            for i in range(self.__nObst):
              self.prob.addConstraint(pulp.lpSum([self.z[k][i][j] for j in range(self.__dObst)]) == \
                                       self.__dObst-1, name='z_%d_%d'%(k, i))
        return

    def __addSCIPVarConstraint(self, zInit):
        '''
        Add constraints on the variables (Includes state transition constraint)
        '''
        # Constraints on state parameters
        # x[0] == xInit 
        count=0
        for x_var, xi in zip(self.x[0].values(), self.xInit):
          self.prob.addCons(x_var == xi, name='constraint%d'%count)
          count = count + 1

        for x_var, xi in zip(self.x[self.__N].values(), self.xT):
          self.prob.addCons(x_var == xi, name='constraint%d'%count)
          count = count + 1

        # Constraints on intermediate variables
        if self.receding_horizon:
            limit = self.__N-1
        else:
            limit = self.__N

        for k in range(limit): 
            # absu >= u
            # absu + u >= 0
            for i in range(self.__nu):
              self.prob.addCons(self.absu[k][i] - self.u[k][i] >= 0, name='constraint%d'%count)
              count = count + 1
              self.prob.addCons(self.absu[k][i] + self.u[k][i] >= 0, name='constraint%d'%count)
              count = count + 1

            # State Transition modelled as a constraint
            # x[k+1] == A*x[k] + B*u[:,k]
            for x_var, a, b in zip(self.x[k+1].values(), self.__A, self.__B):
                cns1 = 0
                for ai, xi in zip(a, self.x[k].values()):
                    cns1 += ai*xi

                cns2 = 0
                for bi, ui in zip(b, self.u[k].values()):
                    cns2 += bi*ui

                self.prob.addCons((x_var - cns1 - cns2) == 0, name='constraint%d'%count)
                count = count + 1

        # Lower bound on the horizon radius
        if self.receding_horizon:
            self.prob.addCons(self.dlast >= 0, name='constraint%d'%count)
            count = count + 1

        # Constraints the distrance between adjacent points
        for kk in range(1, self.__N+1-self.receding_horizon):
            currx = self.x[kk].values()
            prevx = self.x[kk-1].values()
            self.prob.addCons(self.d[kk-1] >= 0, name='constraint%d'%count)
            count = count + 1
            for side in range(self.poly_nsides):
                # parameters that determine the side of the polygon
                line_angle = [math.cos(2*side*math.pi/self.poly_nsides), \
                                        math.sin(2*side*math.pi/self.poly_nsides)]

                # Add constraints between the last step and the goal point
                # This ensures that the goal is within the horizon of the last step
                cns = 0
                for m, x1, x2 in zip(line_angle, currx, prevx):
                    cns += m*(x1-x2)
                self.prob.addCons(cns <= self.d[kk-1], name='constraint%d'%count)
                count = count + 1

        if self.receding_horizon:
            xlaststep = self.x[self.__N-1].values()
            xgoal    = self.x[self.__N].values()
            for side in range(500):
                # parameters that determine the side of the polygon
                line_angle = [math.cos(2*side*math.pi/self.poly_nsides), \
                                        math.sin(2*side*math.pi/self.poly_nsides)]

                # Add constraints between the last step and the goal point
                # This ensures that the goal is within the horizon of the last step
                cns = 0
                for m, x1, x2 in zip(line_angle, xgoal, xlaststep):
                    cns += m*(x1-x2)

                self.prob.addCons(cns <= self.dlast, name='constraint%d'%count)
                count = count + 1

    def __addVarConstraint(self, zInit):
        '''
        Add constraints on the variables (Includes state transition constraint)
        '''
        # Constraints on state parameters
        # x[0] == xInit 
        for x_var, xi in zip(self.x[0].values(), self.xInit):
          self.prob.addConstraint(x_var == xi)

        for x_var, xi in zip(self.x[self.__N].values(), self.xT):
          self.prob.addConstraint(x_var == xi)

        if False: #if self.margin != None:
            # Add the boundary constraints 
            xmin = min(self.xInit[0], self.xT[0]) - self.margin
            xmax = max(self.xInit[0], self.xT[0]) + self.margin
            ymin = min(self.xInit[1], self.xT[1]) - self.margin
            ymax = max(self.xInit[1], self.xT[1]) + self.margin

            for idx in range(1, self.__N):
                for x_var, xm in zip(self.x[idx].values(), [xmin, ymin]):
                    self.prob.addConstraint(x_var >= xm)
                for x_var, xm in zip(self.x[idx].values(), [xmax, ymax]):
                    self.prob.addConstraint(x_var <= xm)

        # Constraints on intermediate variables
        if self.receding_horizon:
            limit = self.__N-1
        else:
            limit = self.__N

        for k in range(limit): 
            # absu >= u
            # absu + u >= 0
            for i in range(self.__nu):
              self.prob.addConstraint(self.absu[k][i] - self.u[k][i] >= 0)
              self.prob.addConstraint(self.absu[k][i] + self.u[k][i] >= 0)

            # State Transition modelled as a constraint
            # x[k+1] == A*x[k] + B*u[:,k]
            for x_var, a, b in zip(self.x[k+1].values(), self.__A, self.__B):
                self.prob.addConstraint((x_var - pulp.lpSum([(ai * xi) for ai, xi in zip(a, self.x[k].values())]) \
                            - pulp.lpSum([(bi * ui) for bi, ui in zip(b, self.u[k].values())])) == 0)

        # Lower bound on the horizon radius
        if self.receding_horizon:
            self.prob.addConstraint(self.dlast >= 0)

        # Constraints the distrance between adjacent points
        for kk in range(1, self.__N+1-self.receding_horizon):
            currx = self.x[kk].values()
            prevx = self.x[kk-1].values()
            self.prob.addConstraint(self.d[kk-1] >= 0)
            for side in range(self.poly_nsides):
                # parameters that determine the side of the polygon
                line_angle = [math.cos(2*side*math.pi/self.poly_nsides), \
                                        math.sin(2*side*math.pi/self.poly_nsides)]

                # Add constraints between the last step and the goal point
                # This ensures that the goal is within the horizon of the last step
                self.prob.addConstraint(pulp.lpSum([m*(x1-x2) for m, x1, x2 in zip(line_angle, currx, prevx)]) <= self.d[kk-1])

        if self.receding_horizon:
            xlaststep = self.x[self.__N-1].values()
            xgoal    = self.x[self.__N].values()
            for side in range(500):
                # parameters that determine the side of the polygon
                line_angle = [math.cos(2*side*math.pi/self.poly_nsides), \
                                        math.sin(2*side*math.pi/self.poly_nsides)]

                # Add constraints between the last step and the goal point
                # This ensures that the goal is within the horizon of the last step
                self.prob.addConstraint(pulp.lpSum([m*(x1-x2) for m, x1, x2 in zip(line_angle, xgoal, xlaststep)]) <= self.dlast)

        # \sum_{i} z_{i} == dim(z_{i}) - 1 constraint
        for k in range(limit): 
            for i in range(self.__nObst):
              self.prob.addConstraint(pulp.lpSum([self.z[k][i][j] for j in range(self.__dObst)]) == \
                                       self.__dObst-1, name='z_%d_%d'%(k, i))

    def getUPoints(self):
        # Get the solution for the control inputs
        usol = []
        for stepnum in range(self.__N): 
          cusol = []
          for unum in range(self.__nu):
            if usePuLP:
              cusol.append(pulp.value(self.u[stepnum][unum]))
            else:
              cusol.append(self.prob.getVal(self.u[stepnum][unum]))
          usol.append(cusol)
        return usol

    def getZsol(self):
        '''
        Get the solution for z variable
        '''
        zsol = {}
        for stepnum in range(self.__N): 
            for obstnum in range(self.__nObst):
                zval = []
                for j in range(self.__dObst):
                    zval.append(pulp.value(self.z[stepnum][obstnum][j]))

                zzval = [np.around(x) for x in zval]

                # get the side being used
                try:
                  cside = zzval.index(0)
                except:
                  cside = -1

                # Create the dictionary
                zsol[(stepnum,obstnum)] = cside

        return zsol

    def getZValues(self):
        '''
        Get the solution for z variable
        '''
        zsol = []
        for stepnum in range(self.__N): 
            for obstnum in range(self.__nObst):
                zval = []
                for j in range(self.__dObst):
                    zval.append(pulp.value(self.z[stepnum][obstnum][j]))

                zzval = [np.around(x) for x in zval]

                # get the side being used
                try:
                  cside = zzval.index(0)
                except:
                  cside = -1

                # Create the dictionary
                zsol.append(cside/4)

        return zsol

    def __addSCIPObstPt(self, xidx, kidx, delta):
        '''
        Adds obstacle constraint for each way point
        '''
        x = self.x[xidx].values()[:2]
        threshold = self.u_max*np.sqrt(2)

        # H*x[k+1] - M*z[k] <= G 
        nConstraint = 0
        for kk, (mDelta, Ci, Ri, Vi) in enumerate(zip(delta, self.__C, self.__R, self.__V)):
            # For each obstacle
            cns=0
            for xi, ci in zip(x, Ci):
                cns += (xi-ci)**2

            if 'constraint%d_%d_%d'%(nConstraint, kidx, xidx) in self.cnstMap.keys():
                self.prob.delCons(self.cnstMap['constraint%d_%d_%d'%(nConstraint, kidx, xidx)])

            assert mDelta[0] >= 0, "Should be positive"
            cnx = self.prob.addCons(cns >= (Ri + mDelta[0])**2 , name='constraint%d_%d_%d'%(nConstraint, kidx, xidx))
            self.cnstMap['constraint%d_%d_%d'%(nConstraint, kidx, xidx)] = cnx
            nConstraint = nConstraint + 1

            #import pdb; pdb.set_trace()

    def __addObstPt(self, zidx, xidx, delta):
        '''
        Adds obstacle constraint for each way point
        '''
        z = self.z[zidx]
        x = self.x[xidx].values()[:2]
        threshold = self.u_max*np.sqrt(2)

        # H*x[k+1] - M*z[k] <= G 
        nConstraint = 0
        for mDelta, Hi, Gi, Zi, Vi in zip(delta, self.__H, self.__G, z.values(), self.__V):
            # For each obstacle
            for m, h, g, zi in zip(mDelta, Hi, Gi, Zi.values()):

                # For each hyperplane of the obstacle
                if (str('constraint%d_%d_%d'%(nConstraint, zidx, xidx)) in self.prob.constraints): 
		    # if key already exists (http://www.coin-or.org/PuLP/pulp.html)
                    # it still needs to be updated 
		    # (that's what were doing -- overwriting the old key with the new data, so
                    del self.prob.constraints['constraint%d_%d_%d'%(nConstraint, zidx, xidx)] 
		    # delete the old key first, then add the new one below
                    # this gets rid of the pulp 
                    # "('Warning: overlapping constraint names:', 'constraint43_19')" 
                    # types of message-printouts 
                    # (http://pulp.readthedocs.org/en/latest/user-guide/troubleshooting.html)

                self.prob.addConstraint((pulp.lpSum([((-hi)*xi) for hi, xi in zip(h,x)]) \
                                             - self.__M*zi + m + g) <= 0,
                                             name='constraint%d_%d_%d'%(nConstraint, zidx, xidx))

                # Used for naming the constraints to replace on recursive calls with delta
                nConstraint = nConstraint + 1

    def writeOut(self, name):
        '''
        Write out the solution
        '''
        if usePuLP:
          self.prob.writeLP(name + '.lp')
        else:
          self.prob.writeProblem(name + '.mps')
        return

    def __addObstConstraint(self, mdelta=None):
        '''
        Adds obstacle constraints based on the H and G matrix to all points
        '''
        num = self.__N - int(self.receding_horizon)
        if mdelta is None:
            mdelta = np.zeros((num, self.__H.shape[0], self.__H.shape[1]))
            mdelta = [mdelta, np.zeros((num-1, self.__H.shape[0], self.__H.shape[1]))]

        # Initial state constraint
        zeroDelta = np.zeros((self.__H.shape[0], self.__H.shape[1]))
        if usePuLP:
          self.__addObstPt(0, 0, zeroDelta)
        else:
          self.__addSCIPObstPt(0, 0, zeroDelta)

        for k in range(num-1): 
            # Adding constraints on the obstacle for each point
            if usePuLP:
                self.__addObstPt(k, k+1, mdelta[0][k])
                self.__addObstPt(k+1, k+1, mdelta[1][k])
            else:
                self.__addSCIPObstPt(k, k+1, mdelta[0][k])
                self.__addSCIPObstPt(k+1, k+1, mdelta[1][k])

        # Last step 
        k = num-1
        if usePuLP:
          self.__addObstPt(k, k+1, mdelta[0][k])
        else:
          self.__addSCIPObstPt(k, k+1, mdelta[0][k])
        return

    def activeObstWayPt(self, x, mDelta, Hi, Gi, Zi):
        '''
        Test is the way points is active for a particular waypoint
        '''
        for m, h, g, z in zip(mDelta, Hi, Gi, Zi):
             hDotx = np.sum([hi*xi for hi, xi in zip(h, x)])

             # To acount for floating point inaccuracy
             if ((hDotx - self.__M*z - m - g) >= 0):
                return True
     
        return False

    #def getActiveBoundaries(self):
    #    '''
    #    Looks at both the active edges 
    #    chooses the one that is closest as the active edges
    #    Because now each z corresponds to 2 state vectors
    #    '''
    #    active_boundaries = [[None for i in range(self.__nObst)]\
    #                            for j in range(self.__N)]

    #    active_dist = self.measureDist()
    #    for t_i in range(self.__N):
    #      for obs_j in range(self.__nObst):
    #          idx = np.where(active_dist[:, t_i, obs_j] < 0)[0]
    #          active_boundaries[t_i][obs_j] = idx[np.argmax(active_dist[idx, t_i, obs_j])]

    #    return active_boundaries

    def getActiveBoundaries(self):
        '''
        Check the active boundaries at each time point for each obstacle
        '''
        num = self.__N-self.receding_horizon
        active_boundaries = [[None for i in range(self.__nObst)]\
                                for j in range(num)]

        for t_i in range(num):
            for obs_j in range(self.__nObst):
                for dobs_k in range(self.__dObst):
                    if pulp.value(self.z[t_i][obs_j][dobs_k]) < 1:
                        
                        ## debug
                        # print 'Z: ' +  str(self.z[t_i][obs_j][dobs_k])
                        # print 'Z value' + str(pulp.value(self.z[t_i][obs_j][dobs_k]))
                        # wp = self.wayPoint[t_i][:2]                        
                        # input("Press Enter to continue...")
                        ##debug

                        active_boundaries[t_i][obs_j] = dobs_k
                        break

        return np.array(active_boundaries)

    def measureDist(self):
        '''
        Adds obstacle constraints based on the H and G matrix to all points
        '''
        active_distance = [[[None for i in range(self.__nObst)]\
                                    for j in range(self.__N)] for k in range(self.__dObst)]

        # Initial point
        zeroDelta = np.zeros((self.__H.shape[0], self.__H.shape[1]))

        for k in range(self.__N): 
            self.__measureDist(k+1, active_distance)
            #self.__measureDist(k+1, k+1, active_distance[1])

        return np.array(active_distance)

    def __measureDist(self, xidx, dist):
        '''
        Adds obstacle constraint for each way point
        '''
        x = self.x[xidx].values()[:2]

        # H*x[k+1] - M*z[k] - G 
        nConstraint = 0
        for j, (Hi, Gi) in enumerate(zip(self.__H, self.__G)):
            # For each obstacle
            for n, (h, g) in enumerate(zip(Hi, Gi)):
               dist[n][xidx-1][j] = np.sum([((-hi)*pulp.value(xi)) for hi, xi in zip(h,x)]) + g

        return

    def activewayPoint(self, idx, delta, mask=None):
        '''
        Checks if waypoint with index is active
        '''
        z = self.__zVal[idx]
        x = self.wayPoint[idx+1][:2]

        if mask is None:
            mask = [False for i in  range(self.__nObst)]

        # H*x[k+1] - M*z[k] <= G 
        for j, (mDelta, Hi, Gi, Zi, cMask) in enumerate(zip(delta, self.__H, self.__G, z, mask)):

            # Mask is used to look at only the obstacles not already tested for delta update
            if cMask is False:
                if self.activeObstWayPt(x, mDelta, Hi, Gi, Zi) is False:
                    continue
                else:
                    return j
            else:
                # Obstacle already parsed
                continue     

        return None

    def addObstacles(self, N, H, G, V, zInit=[]):
        '''
        Adds obstacles with H and G. 
        This function initiates all the constraints for optimization
        '''
        # create problem variables
        self.__N      = N
        self.__createVar()

        nObst         = len(G)
        self.__H      = H
        self.__G      = G
        self.__V      = V
        self.__C      = np.mean(V, axis=1)
        diff = np.square(V[-1] - np.repeat([self.__C[-1]], 4, axis=0))
        radius        = np.sqrt(np.sum(diff, axis=1))
        self.__R      = [radius[0]]*len(self.__C)

        self.__nObst  = self.__G.shape[0]
        self.__dObst  = self.__G[0].shape[0]
        
        # z variable dim (nPoints x nObst)
        num = self.__N - self.receding_horizon
        self.__addObjective()
        if usePuLP:
          self.z = pulp.LpVariable.dicts("z", (range(num), range(self.__nObst), \
  					range(self.__dObst)), 0, 1, pulp.LpInteger)
          self.__addVarConstraint(zInit)
        else:
          self.__addSCIPVarConstraint(zInit)
        self.__addObstConstraint()

    def solve(self, mdelta=None, outF=None, create=False):
        '''
        Solves and extracts the output variables into xVal and yVal
        '''
        # Modify the constraints with new delta
        if mdelta != None:
          self.__addObstConstraint(mdelta)
        
        if create:
          return

        # Solve the optimization problem
        with tempfile.NamedTemporaryFile() as fp:
          # using temporary file to write log
          if usePuLP:
            self.prob.solve(pulp.GUROBI_CMD(msg=self.msg, options={"ResultFile": 'temp.sol', 
                                                                   "NodeLimit": 200000}))
          else:
            self.prob.optimize()
  
          ## Read log file to get number of nodes explored
          #fp.seek(0)
          #data = fp.read()

          ## Read the explored nodes
          #self.exploreNodes = re.findall("Explored (\d+) nodes", data)[0]
          #self.exploreNodes = 0
          #if self.gap == None:
          #  # Do this only for the first time
          #  self.gap = re.findall("gap (\d+.\d+)", data)[0]

        #self.prob.solve(pulp.GUROBI_CMD(msg=self.msg, options={}))
        self.msg = 0

        # Populate the solution variable
        # print("Status: ", pulp.LpStatus[self.prob.status])
        if usePuLP:
          if 'Optimal' in pulp.LpStatus[self.prob.status]:
            self.feasible = True
          else:
            self.feasible = False
        else:
          if 'optimal' in self.prob.getStatus():
            self.feasible = True
          else:
            self.feasible = False

        # Get solution from waypoints
        if self.feasible:
           self.wayPoint = []
           for i in range(self.__N + 1):
              if usePuLP:
                self.wayPoint.append([pulp.value(self.x[i][0]), pulp.value(self.x[i][1])])  
              else:
                self.wayPoint.append([self.prob.getVal(self.x[i][0]), self.prob.getVal(self.x[i][1])])  

           if usePuLP:
              self.__zVal = []
              for i in range(self.__N-self.receding_horizon):
                 self.__zVal.append([[pulp.value(self.z[i][j][k]) for k in range(self.__dObst)] \
	   			       for j in range(self.__nObst)])

              self._sol = {}
              for var in self.prob._variables:
                self._sol[str(var)] = pulp.value(var) 

           if outF is not None:
	     with open(outF + '.yaml', 'w') as outfile:
	        Y.dump(self._sol, outfile, default_flow_style=False, explicit_start=True)

           return True
	else:
           return False

    def plot(self, fname):
        fig = plt.figure()
        ax = fig.add_subplot(111, aspect='equal')
      
        # Plot obstacles
        for g in self.__G:
            ax.add_patch(
                patches.Rectangle(
                    (g[0], g[1]),
                    np.abs(g[0] + g[2]),
                    np.abs(g[1] + g[3]),
                    alpha=0.5)
                )

        # Plot Waypoints
        for pt1, pt2 in zip(self.wayPoint[:-1], self.wayPoint[1:]):
            ax.plot([pt1[0], pt2[0]], [pt1[1], pt2[1]], c='b')
            ax.scatter(pt1[0], pt1[1], c='g')
            ax.scatter(pt2[0], pt2[1], c='g')

        plt.savefig('./' + str(fname) + '.png')
        plt.close('all')

    # Class Interface functions
    def getWayPoints(self):
        return self.wayPoint

    def getActivePoints(self):
        return (self.activePtsIdx, self.activeObstIdx)

    def getHorizonRadius(self):
        if usePuLP:
          return pulp.value(self.dlast)
        else:
          return self.prob.getVal(self.dlast)

    def getObjective(self):
        if usePuLP:
          return pulp.value(self.prob.objective)
        else:
          return self.prob.getObjVal()

class IRA(pSulu):
    '''
    Iterative Risk Allocaion 
    '''
    def __init__(self, configFile):
        '''
        Initialize from the config file - For compatibility with Claudio's pSulu Input
        '''
        global usePuLP

        # Initializing base class
        super(self.__class__, self).__init__(configFile)
        usePuLP = not self.quad

        # Transition parameters
        self.__N            = self.waypoints
        self.__A, self.__B  = self.__initTransformationParam()
        self.__Q, self.__P0 = self.__initCovarianceParam()
        self.__coVarMat     = self.computeCov()
        self.maxCovar = self.__coVarMat[-1].max()
        self.nNodes   = None

        # Output file names
        self.outTreeF = self.outFolder + '/' + os.path.basename(self.environment) + '.gpickle'
        self.plotF    = os.path.basename(self.environment)

        if self.cleanMap:
          self.clean(self.maxCovar + self.cleanDist)

        # pSulu Parameters
        self.alpha  = 0.8
 
        # Obstacles related variables
        self.__H      = self.obstMap.obstNormal
        self.__G      = self.obstMap.obstOffset
        self.__V      = self.obstMap.obstVert
        self.__nObst  = self.obstMap.nObstacles
        self.__zInit  = self.obstMap.zInit

        self.__delta      = self.__initDelta()
        self.__wayPoints  = []
        self.__J          = []
        self.__deltaStep  = 10**(-3)
        self.mask         = None
        self.zsol         = None

        # Radius of the horizon upon solving
        self.horizon_radius = None

	# Chance constraint condition
        #if ((self.chance_constraint > 0.5) or (self.chance_constraint < 0)):
        #    raise Exception('Chance Constraint should be between 0 and 0.5 for convergence theoretical guarantees')

        # Formalizing the MILP solver
        u_max     = self.max_velocity
        self.MILP = PathSolver(u_max, self.start_location, self.end_location, \
                               self.__A, self.__B, self.receding_horizon, \
                               margin=self.margin,
                               poly_nsides=self.horizon_polygon_size, \
                               horizon_cost=self.horizon_cost_proportion)
        self.MILP.addObstacles(self.__N, self.__H, self.__G, self.__V, self.__zInit)
        self.M = self.transformDelta()

    @staticmethod
    def __initTransformationParam(dt=1):
        # Optimization variables
        # Transition Parameters
        A = [[1, 0, 0, 0],
            [0, 1, 0, 0],
            [0, 0, 0, 0],
            [0, 0, 0, 0]]
        B = [[dt/2, 0], 
            [0, dt/2], 
            [0, 0],
            [0, 0]]
        return (A,B)
    
    def writeOut(self, name):
        '''
        Write out the solution
        '''
        self.MILP.writeOut(self.outFolder + '/' + name)
        return

    def get_HG_data(self):
      return self.__H.flatten().tolist() + self.__G.flatten().tolist()

    def get_envi_data(self):
      '''
      Get the Problem specific data 
      '''
      #from extractFeats import getFeats
      #  
      ## save obstacle map as an image
      #fname = self.plotObst()

      ## extract feats from the obstacle map
      #feats = getFeats(fname)
      HGRep = self.__H.flatten().tolist() + self.__G.flatten().tolist()
      return {'V': self.__V, 'HG': HGRep}

    def __initCovarianceParam(self):
        P0 = np.eye(len(self.__A[0])) 
        P0[0,0] = 0 #self.coVarX
        P0[1,1] = 0 #self.coVarY
        P0[2,2] = 0
        P0[3,3] = 0
        Qstate = np.eye(int(len(self.__A[0])/2))
        Qstate[0, 0] = self.coVarX
        Qstate[1, 1] = self.coVarY
        Qzeros = np.zeros([int(len(self.__A[0])/2), int(len(self.__A[0])/2)])
        Q = np.bmat([[Qstate, Qzeros], [Qzeros, Qzeros]])
        Q = np.array(Q)
        return (Q,P0)

    def readObstMap(self, obstMap):
        '''
        Reads the H and G matrix that define the obstacle 
        '''
        import scipy.io as SIO
        matContent = SIO.loadmat(obstMap)

        # Obstacles
        H = np.rollaxis(matContent['obstMap']['H'][0][0], 2)
        G = matContent['obstMap']['g'][0][0]
        V = np.rollaxis(matContent['obstMap']['V'][0][0], 2)
        return (H, G, V)
    
    def getNumSteps(self):
        # Number of steps
        return self.__N

    def getNumObst(self):
        # Number of obstacles 
        return self.__nObst

    def computeExpertSol(self):
        # Add all z variable constraints
        self.MILP.addAllZConstraints()

        # Solve the entire problem to get the solution path
        self.feasible = self.MILP.solve(self.M)

        # Get the optimal path and expand the tree on the optimal path
        if self.feasible:
          # Get way points
          self.__wayPoints = self.MILP.getWayPoints()

          # plot
          #self.plot(self.plotF)

          # z value solution
          self.zsol = self.MILP.getZsol()

        return

    def getExpertSol(self):
        # Compute Expert solution
        self.computeExpertSol()
        return self.zsol

    def create(self):
        self.M = self.transformDelta()
        M_ = self.transformDelta(1)
        self.M = [self.M, M_]
        self.MILP.solve(self.M, create=True)
        return self.MILP.prob

    def solve(self, create=False):
        '''
        Iteratively calls MILP with different delta
        '''
        # Iterate until convergence 
        J = float('inf')
        itr = 1
        while True:
          # Solve the optimization with new delta
          oldJ = J
          self.M = self.transformDelta()
          M_ = self.transformDelta(1)
          self.M = [self.M, M_]
          self.feasible = self.MILP.solve(self.M, create=create)

          #if self.nNodes == None:
          #  self.nNodes = self.MILP.exploreNodes
          #else:
          #  self.nNodes = self.nNodes + self.MILP.exploreNodes
          
          #self.feasible = self.MILP.solve(M, 'sol'+str(itr))
          
          # Write the optimization problem out
          self.MILP.writeOut('MILP'+str(itr))
          if create:
            return

          if not self.feasible: 
             print('MILP Infeasible')
             return
 
          J = self.MILP.getObjective()

          # Saving way points
          self.__J.append(J)
          self.__wayPoints.append(self.MILP.getWayPoints())

          if self.receding_horizon:
            self.horizon_radius = self.MILP.getHorizonRadius()

          self.plot(str(itr) + '.png')
          print("Objective Function Value: %f"%J)

          if not usePuLP:
              break

          if (abs((oldJ - J)/oldJ) < (10**-3)):
            break 
          else:
            # Compute Residue for the delta
            (nActive, deltaResidual, newDelta) = self.__calcDeltaResidue(self.M)
            if nActive > 0:
              riskInc = deltaResidual/nActive
              self.__delta = self.__reallocate(newDelta, riskInc)
            else:
              _num = self.__delta.shape[0] * self.__delta.shape[1]
              riskInc = (deltaResidual/_num)
              self.__delta = newDelta + riskInc

          # Iteration count
          itr = itr + 1
          break

    def __reallocate(self, newDelta, riskInc):
      '''
      Reallocated the risk to active points
      '''
      idx = np.where(np.equal(newDelta, None))
      for iTime, cObst in zip(idx[0], idx[1]):
           _newDelta_ = self.__delta[iTime][cObst] + riskInc

           # Clips the delta between 0 to 0.5 for theoretical guarantees
           newDelta[iTime][cObst] = max(0, min(_newDelta_, 0.5))

      return newDelta
      
    def __calcDeltaResidue(self, M):
      '''
      Computes the Delta Residue to redistribute
      '''
      num = self.__N-self.receding_horizon
      newDelta      = [[None for x in range(self.__H.shape[0])] for y in range(num)]
      usedDelta     = [[None for x in range(self.__H.shape[0])] for y in range(num)]
      deltaResidual = 0
      nActive       = self.__H.shape[0] * num
 
      # find active constraints
      activeBoundaries = self.MILP.getActiveBoundaries() ## need to do
      waypoints        = self.MILP.getWayPoints()
      
      # For each way point
      for iTime in range(num):
        # For each obstacle
        for cObst in range(self.__nObst):
          # find the equation of the active constraint
          activeIdx = activeBoundaries[iTime][cObst]
          activeH = self.__H[cObst][activeIdx]
          activeG = self.__G[cObst][activeIdx]
   
          # redistribute when risk used is less than allocated, recalculate risk
          # Check the gap for both terminal ends of the edge
          if (iTime != 0) and (iTime < num-1):
              wp1 = waypoints[iTime+1]
              realGap1 = abs(np.dot(activeH,wp1) - M[0][iTime][cObst][activeIdx] - activeG)
              wp2 = waypoints[iTime]
              realGap2 = abs(np.dot(activeH,wp2) - M[1][iTime-1][cObst][activeIdx] - activeG)
              timesteps = [iTime, iTime-1]
              gaps = [realGap1, realGap2]
              timestep = timesteps[np.argmin(gaps)]
              realGap = np.min(gaps)
          elif iTime == num-1:
              wp2 = waypoints[iTime]
              realGap2 = abs(np.dot(activeH,wp2) - M[1][iTime-1][cObst][activeIdx] - activeG)
              timestep = iTime-1
              realGap = realGap2
          else:
              wp1 = waypoints[iTime+1]
              realGap1 = abs(np.dot(activeH,wp1) - M[0][iTime][cObst][activeIdx] - activeG)
              timestep = iTime
              realGap = realGap1
            
          if (abs(realGap) > 0.001):
            usedRisk = self.evaluateRisk(timestep, activeH, activeG)
            newDelta[iTime][cObst] = (self.__delta[iTime][cObst] * self.alpha + \
                                       (1-self.alpha)*usedRisk)
            deltaResidual = deltaResidual + (self.__delta[iTime][cObst] - \
                                            newDelta[iTime][cObst])
            nActive = nActive - 1

      return (nActive, deltaResidual, newDelta)

    def evaluateRisk(self, iTime, activeH, activeG):
        '''
        Evaluates Risk given active H and G for a waypoint
        '''
        # find waypoint
        wp = self.MILP.wayPoint[iTime+1][:2]
        P  = self.__coVarMat[iTime][:2,:2]

        # find affine transform param
        h_P_h = np.dot(activeH,np.dot(P,activeH))        
        safety_margin = np.dot(activeH,wp) - activeG

        if np.all(np.array(P) == 0):
            return 0
              
        # print 'safety ' + str(safety_margin)
        # print 'hPh ' + str(h_P_h)
        return stats.norm.sf(safety_margin,0,np.sqrt(h_P_h))

    def plotObst(self):
        '''
        Plot the obstacle map
        '''
        fig = plt.figure(frameon=False)
        ax = fig.add_subplot(111, aspect='equal')
        ax.set_xlim([0,1])
        ax.set_ylim([0,1])
        ax.axis('off')

        ## Plot obstacles
        for corners in self.__V:
          x = corners[:,0]
          y = corners[:,1]
          plt.fill(x, y, color='black')

        fName = self.outFolder + '/' + self.plotF.split('.')[0] \
                                  + '_obstmap' + '.png'
        fig.canvas.print_png(fName)
        plt.close('all')
        return fName

    def plot(self, fname, ax=None, wayPt=None, numObst=None):
        '''
        Plots all the way points over all iterations
        '''
        from pylab import rand
        from matplotlib.patches import Ellipse, Circle
        if ax == None:
          fig = plt.figure()
          ax = fig.add_subplot(111, aspect='equal')

        colormap = plt.cm.gist_ncar

        ## Different color points 
        clrMap = [colormap(i) for i in np.linspace(0, 0.9, len(self.__wayPoints))]

        if numObst == None:
          numObst = len(self.__V)
      
        ## Plot obstacles
        for corners in self.__V[:numObst]:
          x = corners[:,0]
          y = corners[:,1]
          plt.fill(x, y, color='black')

        ## Plot Waypoints
        if self.feasible or (wayPt != None):
          if wayPt == None:
            wayPt = self.getWayPoints()
          covar = self.__coVarMat

          for i, (pt1, pt2) in enumerate(zip(wayPt[:-1], wayPt[1:])):
            if (i == len(wayPt)-2) and self.receding_horizon:
                # Receding horizon to not plot that last time step
                break

            #ax.plot([pt1[0], pt2[0]], [pt1[1], pt2[1]], c=clrMap[-1])
            ax.plot([pt1[0], pt2[0]], [pt1[1], pt2[1]], c='r')
            ax.scatter(pt1[0], pt1[1], c='g')
            ax.scatter(pt2[0], pt2[1], c='g')
            #if (i > 0) and (i<(len(wayPt) -1)):
            #  ells = Ellipse(np.array(wayPt[i]), width=3*(covar[i-1][0][0]), \
	    #    			height=3*(covar[i-1][1][1]), angle=0)
            #  ax.add_artist(ells)
            #  ells.set_alpha(0.4)
            #  ells.set_facecolor('g')


          if self.receding_horizon:
            pt1 = wayPt[-2]
            pt2 = wayPt[-1]
            ax.plot([pt1[0], pt2[0]], [pt1[1], pt2[1]], linestyle='--', c='r')
            ax.scatter(pt2[0], pt2[1], c='g')

            # Plot the circle around the last way point
            horizon = Circle(wayPt[-2], radius=self.horizon_radius)
            ax.add_artist(horizon)
            horizon.set_alpha(0.2)
            horizon.set_facecolor('g')

        else:
            ax.scatter(self.start_location[0], self.start_location[1], c='g')
            ax.scatter(self.end_location[0], self.end_location[1], c='g')

        if fname != None:
          plt.savefig(self.outFolder + '/' + str(fname) + '.png')
          plt.close('all')

        return

    def plot_objective(self):
        plt.plot(self.__J, 'bo', self.__J, 'k')
        plt.ylabel("Objective Function value")
        plt.xlabel("IRA iteration")
        plt.show()

    def __deltaToDistance(self, idelta, H, idx):
        '''
        Computes the mdelta for each points
        '''        
        coVarMat = self.__coVarMat[idx+1][:2,:2]
        mdelta = []
        for delta, iH in zip(idelta, H): 
            cM = []
            for h in iH:
              if not np.all(coVarMat == 0):
                cM.append(stats.norm.isf(delta, 0, \
				np.sqrt(np.dot(h, np.dot(coVarMat, np.transpose(h))))))
              else:
                cM.append(0)

            mdelta.append(cM)

        return mdelta

    def __deltaGToDistance(self, delta, H):
        '''
        Computes the mdelta for each points
        '''        
        idx = self.__N-1
        coVarMat = self.__coVarMat[idx][:2,:2]

        mdelta = []
        for idelta in delta:
          M = []
          for cdelta, iH in zip(idelta, H): 
              M.append(0)
          mdelta.append(M)

        return mdelta

    def transformDelta(self, offset=0):
        '''
        Computes the m(\delta) 
        '''
        assert offset <= 1, "Only single offset is logical"
        M = []
        H = self.__H
        for idx, idelta in enumerate(self.__delta[offset:]):        
            m = self.__deltaToDistance(idelta, H, idx)
            M.append(m)

        return M

    def __initDelta(self):
        '''
        Delta is the risk factor corresponding to each individual point
        '''
        num = self.__N - self.receding_horizon
        delta = np.ones((num, self.__H.shape[0])) 
        #delta = delta * self.chance_constraint/(num*self.__H.shape[0])
        delta = delta*self.chance_constraint
        return delta

    def computeCov(self):
        '''
        Computes the covariance at each points
        '''
        A = self.__A
        Q = self.__Q
        coVarMat = [self.__P0]
        for idx in range(self.__N):
            cvM = np.dot(A, np.dot(coVarMat[idx], np.transpose(A))) + Q
            coVarMat.append(cvM)

        return coVarMat

    def getWayPoints(self):
        if self.__wayPoints != []:
          if len(self.__wayPoints[-1]) > 2:
            return self.__wayPoints[-1]
          else:
            return self.__wayPoints
        else:
          self.__wayPoints = self.MILP.getWayPoints()

    def isFeasible(self):
        return self.feasible

    def getNextWP(self, xi, ui):
        A = np.array(self.__A)
        B = np.array(self.__B)
        if len(xi) == 2:
            x = np.array(xi + [0, 0]).reshape(len(xi) + 2, 1)
        else:
            x = np.array(xi).reshape(len(xi), 1)

        u = np.array(ui).reshape(len(ui), 1)
        noise = np.zeros((4,1))
        for i in range(4):
            noise[i] = np.random.normal(0, np.sqrt(self.coVarX))

        nextX = np.dot(A, x) + np.dot(B, u) + noise
        return nextX.tolist()

    def writeOutput(self, outF):

        # Write the waypoints to a file
        wp = self.MILP.getWayPoints()
        up = self.MILP.getUPoints()

        # Remove the last way point which is the destination in case of receding horizon
        if self.receding_horizon:
            wp = wp[:-1]

        # Get real state vector
        length = len(wp)-1
        newwp = [wp[0]]
        prevx = wp[0]
        for u in up[:length]:
           newx = self.getNextWP(prevx, u)
           newwp.append([newx[0][0], newx[1][0]])
           prevx = newx

        Y.dump(wp, open(self.outFolder + '/' + outF, 'w'))
        Y.dump(newwp, open(self.outFolder + '/' + outF + '.real', 'w'))
        return

def firstPassCommandLine():
    
    # Creating the parser for the input arguments
    parser = argparse.ArgumentParser(description='Path Planning based on MILP')

    # Positional argument - Input XML file
    #parser.add_argument('-u_max', '--u_max', type=int, default=1.25,
    #                    help='Maximum value of velocity', dest='u_max')
    #parser.add_argument('-nSteps', '--N', type=int, default=20,
    #                    help='Number of steps to the destination', dest='nSteps')
    #parser.add_argument('-i', type=str, default=None,
    #                    help='Input Obstacle Map (.mat file)', dest='obstMap')
    parser.add_argument('-i', type=str, default='./config/param.yaml',
                        help='Input File (Yaml Formal)', dest='inputFile')
    parser.add_argument('-o', type=str, default='path.yaml',
                        help='Output File containing the way points (Yaml Formal)', dest='outputFile')

    # Parse input
    args = parser.parse_args()
    return args

def main(inp, out=None, create=False):
    # Create IRA instance and solve it
    # np.random.seed(0)
    itrRA = IRA(inp)
    itrRA.solve(create=create) 

    if create:
        return

    if itrRA.isFeasible():
        itrRA.plot('output_plot')
        feats = itrRA.get_envi_data()
        with open('feats.pkl', 'wb') as f:
            pickle.dump(feats, f, pickle.HIGHEST_PROTOCOL)

        if out != None:
            itrRA.writeOutput(out)
    else:
        wp = []
        if out != None:
            Y.dump(wp, open(itrRA.outFolder + '/' + out, 'w'))
            Y.dump(wp, open(itrRA.outFolder + '/' + out + '.real', 'w'))

    return itrRA.isFeasible()
        
if __name__ == '__main__':
    args = firstPassCommandLine()
    main(args.inputFile, args.outputFile)
