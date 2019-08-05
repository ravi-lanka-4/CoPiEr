#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "helper.h"
#include "scip/sol.h"
#include "scip/def.h"
#include "scip/visual.h"
#include "nodesel_oracle.h"

extern void getBranchInfo(
   SCIP_NODE*            node,               /**< node */
   SCIP_VAR**            var,                /**< pointer to store the branching variable */
   SCIP_BOUNDTYPE*       boundtype,          /**< pointer to store the branching type: lower or upper bound */
   SCIP_Real*            bound               /**< pointer to store the new bound of the branching variable */
);

/* Counts the number of feasible solutions with the prefix passed as param */
int countnFeasible(
    char* prefix
)
{
   int count=0;
   while(TRUE){
      char fname[SCIP_MAXSTRLEN];
      char idx[SCIP_MAXSTRLEN];

      // Create the name of the solution file
      strcpy(fname, prefix);
      sprintf(idx, "%d", count);
      strcat(fname, idx);
      strcat(fname, ".sol");

      if(access(fname, F_OK) == -1)
            break;
      
      count = count + 1;
   }
   return count;
}

/* Get best objective */
SCIP_Real getBestObj(
    SCIP_SOL**      allsols, 
    int             nsols
)
{
   int i=0;
   SCIP_Real bestobj = SCIP_REAL_MAX;
   for(i=0; i<nsols; i++){
     SCIP_Real cobj;
     assert(allsols[i] != NULL);
     cobj = SCIPsolGetOrigObj(allsols[i]);
     bestobj = (cobj < bestobj)? cobj : bestobj;
   }
   return bestobj;
}

/* Get the best index that the node belongs */
/* It is possible that a node could belong to several different optimal sols */
SCIP_Longint getSolIndex(
    SCIP*           scip,
    SCIP_NODE*      node, 
    SCIP_SOL**      allsols, 
    SCIP_Bool*      solflag,
    int             nsols,
    SCIP_Real       margin
)
{
    int idx=-1, i=0;
    SCIP_Real bestobj, prevbest;
    SCIP_Bool isoptimal;
    bestobj = SCIP_REAL_MAX;
    for(i=0; i<nsols; i++){
      if (solflag[i]){
        assert(allsols[i] != NULL);

        // Check optimality for the node
        isoptimal = SCIPnodeCheckOptimal(scip, node, allsols[i], i, 0, margin);

        if (isoptimal){
            // Update best
            SCIP_Real cobj;
            cobj = SCIPsolGetOrigObj(allsols[i]);
            prevbest = bestobj;
            bestobj = (cobj < bestobj)? cobj : bestobj;

            // Track change
            if (prevbest != bestobj)
                idx = i;
        }
      }
    }

    /* Now set the bounds for the variable */
    setNodeUbLb(node);
    return idx;
} 

/* Solution file */
void getSolFname(char* fname, char* prefix, int count){
   char idx[SCIP_MAXSTRLEN];
   assert(count>=0);

   // Create the name of the solution file
   strcpy(fname, prefix);
   sprintf(idx, "%d", count);
   strcat(fname, idx);
   strcat(fname, ".sol");

   return;
}

SCIP_Bool SCIPnodeSetFeats(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE*            node
   )
{
   /* get parent branch vars lead to this node */

   SCIP_VAR**            branchvars;         /* array of variables on which the branchings has been performed in all ancestors */
   SCIP_Real*            branchbounds;       /* array of bounds which the branchings in all ancestors set */
   SCIP_BOUNDTYPE*       boundtypes;         /* array of boundtypes which the branchings in all ancestors set */
   int                   nbranchvars;        /* number of variables on which branchings have been performed in all ancestors */
   int                   branchvarssize;     /* available slots in arrays */ 
   int                   i = 0;

   /* don't consider root node */
   assert(node != NULL);
   assert(SCIPnodeGetDepth(node) != 0);
   branchvarssize = 1;

   /* memory allocation */
   SCIP_CALL( SCIPallocBufferArray(scip, &branchvars, branchvarssize) );
   SCIP_CALL( SCIPallocBufferArray(scip, &branchbounds, branchvarssize) );
   SCIP_CALL( SCIPallocBufferArray(scip, &boundtypes, branchvarssize) );

   SCIPnodeGetParentBranchings(node, branchvars, branchbounds, boundtypes, &nbranchvars, branchvarssize);

   /* if the arrays were to small, we have to reallocate them and recall SCIPnodeGetParentBranchings */
   if( nbranchvars > branchvarssize )
   {
      branchvarssize = nbranchvars;

      /* memory reallocation */
      SCIP_CALL( SCIPreallocBufferArray(scip, &branchvars, branchvarssize) );
      SCIP_CALL( SCIPreallocBufferArray(scip, &branchbounds, branchvarssize) );
      SCIP_CALL( SCIPreallocBufferArray(scip, &boundtypes, branchvarssize) );

      SCIPnodeGetParentBranchings(node, branchvars, branchbounds, boundtypes, &nbranchvars, branchvarssize);
      assert(nbranchvars == branchvarssize);
   }

   /* check optimality */
   assert(nbranchvars >= 1);
   for( i = 0; i < nbranchvars; ++i)
   {
      int step=-1, dim=-1, side=-1;

      SCIPnodeSetBranchBound(node, branchbounds[i]);
      SCIPnodeSetBoundType(node, boundtypes[i]);

      // Defined for pSulu for now -- due to time constraint and budget limitations
      if (SCIPgetNBinVars(scip) != 0){
        if (SCIPgetNContVars(scip) != 0){
          /* pSulu with binary variables */
          sscanf(SCIPvarGetName(branchvars[i]), "t_z_%d_%d_%d", &step, &dim, &side);
          SCIPnodeSetIndex(node, step, 0);
          SCIPnodeSetIndex(node, dim, 1);
          SCIPnodeSetIndex(node, side, 1);
          assert(step != -1);
          assert(dim != -1);
          assert(side != -1);
        }
        else{
          /* Only binary variables no continuous variables -- mvc */
          sscanf(SCIPvarGetName(branchvars[i]), "t_V%d", &step);
          SCIPnodeSetIndex(node, step, 0);
          assert(step != -1);
        }
      }
      else{
        /* Only continuous variables -- pSulu spatial */
        sscanf(SCIPvarGetName(branchvars[i]), "t_x_%d_%d", &step, &dim);
        SCIPnodeSetIndex(node, step, 0);
        SCIPnodeSetIndex(node, dim, 1);
        assert(step != -1);
        assert(dim != -1);
      }
   }

   /* free all local memory */
   SCIPfreeBufferArray(scip, &branchvars);
   SCIPfreeBufferArray(scip, &boundtypes);
   SCIPfreeBufferArray(scip, &branchbounds);
   return TRUE;
}


void setNodeUbLb(SCIP_NODE* node){
  #define MAX_TMP 1000

  /* fill node features here */
  SCIP_VAR* branchvar;
  SCIP_BOUNDTYPE branchtype;
  SCIP_Real branchbound;
  SCIP_Real lb=MAX_TMP+1, ub=MAX_TMP+1;

  /* get branching information */
  getBranchInfo(node, &branchvar, &branchtype, &branchbound);

  /* Local bounds not updated for new children */
  int ctype = SCIPnodeGetBoundType(node);
  if(ctype == SCIP_BOUNDTYPE_LOWER){
    lb = SCIPnodeGetBranchBound(node);
  }
  else if(ctype == SCIP_BOUNDTYPE_UPPER){
    ub = SCIPnodeGetBranchBound(node);
  }

  if( branchvar != NULL ){
    if (lb >= MAX_TMP)
      lb = SCIPvarGetLbLocal(branchvar);
    else
      lb = max(SCIPvarGetLbLocal(branchvar), lb);

    if (ub >= MAX_TMP)
      ub = SCIPvarGetUbLocal(branchvar);
    else
      ub = min(SCIPvarGetUbLocal(branchvar), ub);
  }

  SCIPnodeSetUbLb(node, ub, lb);

  return;

}

