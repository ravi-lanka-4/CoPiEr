Follow the instructions in the graphnn github
 - cd spatial-policy/graph_comb_opt/
 - Follow the instructions to install Graphnn and mvc_lib : https://github.com/Hanjun-Dai/graphnn

Install scip-4.0.1-affected (in dep/ subfolder):
 - [Optional] (If using open source solver:) https://projects.coin-or.org/Clp
 - [Optional] Use SoPlex; so much faster than Clp
 - Use gurobi-7.4 version
 - Make command: make OPT=opt ZIMPL=false LPS=grb (or) make OPT=opt ZIMPL=false LPS=clp [for clp solver]

Install:
 - Install https://github.com/Reference-LAPACK/lapack
 - Remember: It has several dependencies. Make sure you use the same compiler (gcc/g++-4.9) for the all the dependency packages.

Next install scip-dagger:
 - Install co-training/scip-dagger
 - make OPT=opt ZIMPL=false LPS=grb (or) clp [Use the same command as in step 2]. 

Define these macros in your ~/.bashrc (and remember to restart your terminal) :
  - export COTRAIN_HOME=\<path to local copier folder\>
  - export COTRAIN_DATA=\<path to data\>
  - export COTRAIN_SCATCH=\<path to scratch folder\>
  - (optional) export PYTHONDONTWRITEBYTECODE=1 

