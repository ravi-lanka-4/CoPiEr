/**@file   nodesel_policy.c
 * @brief  node selector using a learned policy 
 * @author He He 
 *
 * the UCT node selection rule selects the next leaf according to a mixed score of the node's actual lower bound
 *
 * @note It should be avoided to switch to uct node selection after the branch and bound process has begun because
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/
#include <assert.h>
#include <string.h>
#include "nodesel_policy.h"
#include "nodesel_oracle.h"
#include "feat.h"
#include "policy.h"
#include "helper.h"
#include "struct_policy.h"
#include "scip/sol.h"
#include "scip/tree.h"
#include "scip/struct_set.h"
#include "scip/struct_scip.h"
#include "struct_nodesel_policy.h"

#define NODESEL_NAME            "policy"
#define NODESEL_DESC            "node selector which selects node according to a policy"
#define NODESEL_STDPRIORITY     10
#define NODESEL_MEMSAVEPRIORITY 0

#define DEFAULT_FILENAME        ""

/*
 * Data structures
 */

void SCIPnodeselpolicyPrintStatistics(
   SCIP*                 scip,
   SCIP_NODESEL*         nodesel,
   FILE*                 file
   )
{
   assert(scip != NULL);
   assert(nodesel != NULL);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, 
         "Node selector      :\n");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, 
         "  selection time   : %10.2f\n", SCIPnodeselGetTime(nodesel));
}

/** solving process initialization method of node selector (called when branch and bound process is about to begin) */
static
SCIP_DECL_NODESELINIT(nodeselInitPolicy)
{
   SCIP_NODESELDATA* nodeseldata;
   assert(scip != NULL);
   assert(nodesel != NULL);

   nodeseldata = SCIPnodeselGetData(nodesel);
   assert(nodeseldata != NULL);

   /* read policy */
#ifdef LIBLINEAR
   SCIP_CALL( SCIPpolicyCreate(scip, &nodeseldata->policy) );
   assert(nodeseldata->polfname != NULL);
   SCIP_CALL( SCIPreadLIBSVMPolicy(scip, nodeseldata->polfname, &nodeseldata->policy) );
   assert(nodeseldata->policy->weights != NULL);
#else
   SCIP_CALL( SCIPkerasPolicyCreate(scip, &nodeseldata->policy) );
   assert(nodeseldata->polfname != NULL);
   SCIP_CALL( SCIPreadKerasPolicy(scip, nodeseldata->polfname, &nodeseldata->policy) );
   assert(nodeseldata->policy != NULL);
#endif

   /* Read problem specific features */
   nodeseldata->probfeats = NULL;
   if(strcmp(nodeseldata->probfeatsfname, DEFAULT_FILENAME) != 0)
   {
      FILE* featsfp= fopen(nodeseldata->probfeatsfname, "r");

      /* Assumes that the first line is the size of the feats */
      fscanf(featsfp, "%d\n", &(nodeseldata->probfeatsize));
      SCIP_ALLOC( BMSallocMemoryArray(&(nodeseldata->probfeats), nodeseldata->probfeatsize));

      /* Read the features */
      readProbFeats(featsfp, nodeseldata->probfeats, nodeseldata->probfeatsize);
      fclose(featsfp);
   }
   else{
      nodeseldata->probfeatsize = 0;
   }

   /* create feat */
   nodeseldata->feat = NULL;
   if(strcmp(nodeseldata->normfname, DEFAULT_FILENAME) != 0){
      FILE* normF = fopen(nodeseldata->normfname, "r");
      SCIP_CALL( SCIPfeatCreate(scip, &nodeseldata->feat, normF,  
                     SCIP_FEATNODESEL_SIZE + nodeseldata->probfeatsize + BUFF, SCIP_FEATNODESEL_SIZE) );
      fclose(normF);
   }
   else{
      SCIP_CALL( SCIPfeatCreate(scip, &nodeseldata->feat, NULL, 
                     SCIP_FEATNODESEL_SIZE + nodeseldata->probfeatsize + BUFF, SCIP_FEATNODESEL_SIZE) );
   }
   assert(nodeseldata->feat != NULL);
   SCIPfeatSetMaxDepth(nodeseldata->feat, SCIPgetNBinVars(scip) + SCIPgetNIntVars(scip));
  
   return SCIP_OKAY;
}

/** destructor of node selector to free user data (called when SCIP is exiting) */
static
SCIP_DECL_NODESELEXIT(nodeselExitPolicy)
{
   SCIP_NODESELDATA* nodeseldata;
   assert(scip != NULL);
   assert(nodesel != NULL);

   nodeseldata = SCIPnodeselGetData(nodesel);

   assert(nodeseldata->feat != NULL);
   SCIP_CALL( SCIPfeatFree(scip, &nodeseldata->feat) );

#ifdef LIBLINEAR
   assert(nodeseldata->policy != NULL);
   SCIP_CALL( SCIPpolicyFree(scip, &nodeseldata->policy) );
#else
   assert(nodeseldata->policy != NULL);
   SCIP_CALL( SCIPkerasPolicyFree(scip, &nodeseldata->policy) );
#endif

   if(nodeseldata->probfeats != NULL)
      BMSfreeMemory(&(nodeseldata->probfeats));

   return SCIP_OKAY;
}

/** destructor of node selector to free user data (called when SCIP is exiting) */
static
SCIP_DECL_NODESELFREE(nodeselFreePolicy)
{
   SCIP_NODESELDATA* nodeseldata;

   nodeseldata = SCIPnodeselGetData(nodesel);
   assert(nodeseldata != NULL);

   SCIPfreeBlockMemory(scip, &nodeseldata);

   SCIPnodeselSetData(nodesel, NULL);

   return SCIP_OKAY;
}

/** node selection method of node selector */
static
SCIP_DECL_NODESELSELECT(nodeselSelectPolicy)
{
   SCIP_NODESELDATA* nodeseldata;
   SCIP_NODE** children;
   SCIP_Real noise;
   int nchildren;
   int i;

   assert(nodesel != NULL);
   assert(strcmp(SCIPnodeselGetName(nodesel), NODESEL_NAME) == 0);
   assert(scip != NULL);
   assert(selnode != NULL);

   nodeseldata = SCIPnodeselGetData(nodesel);
   assert(nodeseldata != NULL);

   /* collect leaves, children and siblings data */
   SCIP_CALL( SCIPgetChildren(scip, &children, &nchildren) );

   /* check newly created nodes */
   for( i = 0; i < nchildren; i++)
   {
      /* extract node specific features */
      SCIPnodeSetFeats(scip, children[i]);
      setNodeUbLb(children[i]);

      /* compute score */
      SCIPcalcNodeselFeat(scip, children[i], nodeseldata->feat, \
                                    nodeseldata->probfeats, nodeseldata->probfeatsize);

#ifdef LIBLINEAR
      SCIPcalcNodeScore(children[i], nodeseldata->feat, nodeseldata->policy, nodeseldata->probfeatsize);
#else
      SCIPcalcKerasNodeScore(children[i], nodeseldata->feat, &nodeseldata->policy, nodeseldata->probfeatsize);
#endif

      if (nodeseldata->score_noise != 0){
         noise = getRandom(nodeseldata->score_noise);
         SCIPnodeSetScore(children[i], SCIPnodeGetScore(children[i]) + noise);
      }
   }

   *selnode = SCIPgetBestNode(scip);

   return SCIP_OKAY;
}

/** node comparison method of policy node selector */
static
SCIP_DECL_NODESELCOMP(nodeselCompPolicy)
{  /*lint --e{715}*/
   SCIP_Real score1;
   SCIP_Real score2;

   assert(nodesel != NULL);
   assert(strcmp(SCIPnodeselGetName(nodesel), NODESEL_NAME) == 0);
   assert(scip != NULL);

   score1 = SCIPnodeGetScore(node1);
   score2 = SCIPnodeGetScore(node2);

   assert(score1 != 0);
   assert(score2 != 0);

   if( SCIPisGT(scip, score1, score2) )
      return -1;
   else if( SCIPisLT(scip, score1, score2) )
      return +1;
   else
   {
      int depth1;
      int depth2;

      depth1 = SCIPnodeGetDepth(node1);
      depth2 = SCIPnodeGetDepth(node2);
      if( depth1 > depth2 )
         return -1;
      else if( depth1 < depth2 )
         return +1;
      else
      {
         SCIP_Real lowerbound1;
         SCIP_Real lowerbound2;

         lowerbound1 = SCIPnodeGetLowerbound(node1);
         lowerbound2 = SCIPnodeGetLowerbound(node2);
         if( SCIPisLT(scip, lowerbound1, lowerbound2) )
            return -1;
         else if( SCIPisGT(scip, lowerbound1, lowerbound2) )
            return +1;
         else
            return 0;
      }
   }
}

/*
 * node selector specific interface methods
 */

/** creates the uct node selector and includes it in SCIP */
SCIP_RETCODE SCIPincludeNodeselPolicy(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_NODESELDATA* nodeseldata;
   SCIP_NODESEL* nodesel;

   /* create policy node selector data */
   SCIP_CALL( SCIPallocBlockMemory(scip, &nodeseldata) );

   nodesel = NULL;
   nodeseldata->polfname = NULL;
   nodeseldata->normfname = NULL;
   nodeseldata->probfeatsfname = NULL;

   /* use SCIPincludeNodeselBasic() plus setter functions if you want to set callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeNodeselBasic(scip, &nodesel, NODESEL_NAME, NODESEL_DESC, NODESEL_STDPRIORITY,
          NODESEL_MEMSAVEPRIORITY, nodeselSelectPolicy, nodeselCompPolicy, nodeseldata) );

   assert(nodesel != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetNodeselCopy(scip, nodesel, NULL) );
   SCIP_CALL( SCIPsetNodeselInit(scip, nodesel, nodeselInitPolicy) );
   SCIP_CALL( SCIPsetNodeselExit(scip, nodesel, nodeselExitPolicy) );
   SCIP_CALL( SCIPsetNodeselFree(scip, nodesel, nodeselFreePolicy) );

   /* add policy node selector parameters */
   SCIP_CALL( SCIPaddStringParam(scip, 
         "nodeselection/"NODESEL_NAME"/polfname",
         "name of the policy model file",
         &nodeseldata->polfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodeselection/"NODESEL_NAME"/normfname",
         "name of the normalization file",
         &nodeseldata->normfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip, 
         "nodeselection/"NODESEL_NAME"/score_noise",
         "Noise added to the score of each node", 
         &nodeseldata->score_noise, FALSE, 0, 0, 1, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodeselection/"NODESEL_NAME"/probfeatsfname",
         "name of the file with prob specific features",
         &nodeseldata->probfeatsfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );

   return SCIP_OKAY;
}
