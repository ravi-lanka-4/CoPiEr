Step: Follow the instructions in the graphnn github
 - Don’t need to git clone any repo. All the necessary dependencies are already in the zip.
 - Graphnn: https://github.com/Hanjun-Dai/graphnn

Install scip-4.0.1-affected (in dep/ subfolder):
 - [Optional] (If using open source solver:) https://projects.coin-or.org/Clp
 - [Optional] Use SoPlex; so much faster than Clp
 - Use gurobi-7.4 version
 - Make command: make OPT=opt ZIMPL=false LPS=grb

Install:
 - Install https://github.com/Reference-LAPACK/lapack
 - Remember: It has several dependencies. Make sure you use the same compiler (gcc/g++-4.9) for the all the dependency packages.

Next install scip-dagger:
 - Install co-training/scip-dagger
 - Edit path to scip-4.0.1-affected in the makefile.
 - make OPT=opt ZIMPL=false LPS=grb [Use the same command as in step 2]. 
 - Library and application should be compiled/built with the same command.


Now define there macros in your ~/.bashrc:
  - export COTRAIN_HOME=\<path to local copier folder\>
  - export COTRAIN_DATA=\<path to data\>
  - export COTRAIN_SCATCH=\<path to scratch folder\>
  - (optional) export PYTHONDONTWRITEBYTECODE=1 

