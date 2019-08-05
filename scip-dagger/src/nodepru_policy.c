/**@file   nodepru_policy.c
 * @brief  node pruner which prunes node according a learned policy 
 * @author He He 
 *
 * the UCT node pruning rule pruects the next leaf according to a mixed score of the node's actual lower bound
 *
 * The idea of UCT node pruning for MIP appeared in:
 *
 * The authors adapted a game-tree exploration scheme called UCB to MIP trees. Starting from the root node as current node,
 *
 * The node pruner features several parameters:
 *
 * @note It should be avoided to switch to uct node pruning after the branch and bound process has begun because
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/
#include <assert.h>
#include <string.h>
#include "nodepru_policy.h"
#include "nodepru_oracle.h"
#include "nodesel_oracle.h"
#include "feat.h"
#include "policy.h"
#include "struct_policy.h"
#include "scip/sol.h"
#include "scip/tree.h"
#include "scip/set.h"
#include "scip/clock.h"
#include "scip/struct_set.h"
#include "scip/struct_scip.h"

#define NODEPRU_NAME            "policy"
#define NODEPRU_DESC            "node pruner which pruects node according to a policy but writes exampels according to the oracle"
#define NODEPRU_STDPRIORITY     10
#define NODEPRU_MEMSAVEPRIORITY 0

#define DEFAULT_FILENAME        ""


/*
 * Data structures
 */

/** node pruner data */
struct SCIP_NodepruData
{
   char*              polfname;           /**< name of the solution file */
   char*              normfname;           /**< name of the solution file */
#ifdef LIBLINEAR
   SCIP_POLICY*       policy;
#else
   void *             policy;
#endif
   SCIP_FEAT*         feat;
   int                nprunes;
   SCIP_Real          score_noise;

   /* Problem specific features */
   char*              probfeatsfname;
   SCIP_Real*         probfeats;
   int                probfeatsize;
};

void SCIPnodeprupolicyPrintStatistics(
   SCIP*                 scip,
   SCIP_NODEPRU*         nodepru,
   FILE*                 file
   )
{
   SCIP_NODEPRUDATA* nodeprudata;

   assert(scip != NULL);
   assert(nodepru != NULL);

   nodeprudata = SCIPnodepruGetData(nodepru);
   assert(nodeprudata != NULL);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, 
         "Node pruner        :\n");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, 
         "  nodes pruned     : %10d\n", nodeprudata->nprunes);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, 
         "  pruning time     : %10.2f\n", SCIPnodepruGetTime(nodepru));
}

/** solving process initialization method of node pruner (called when branch and bound process is about to begin) */
static
SCIP_DECL_NODEPRUINIT(nodepruInitPolicy)
{
   SCIP_NODEPRUDATA* nodeprudata;
   assert(scip != NULL);
   assert(nodepru != NULL);

   nodeprudata = SCIPnodepruGetData(nodepru);
   assert(nodeprudata != NULL);

   /* read policy */
#ifdef LIBLINEAR
   SCIP_CALL( SCIPpolicyCreate(scip, &nodeprudata->policy) );
   assert(nodeprudata->polfname != NULL);
   SCIP_CALL( SCIPreadLIBSVMPolicy(scip, nodeprudata->polfname, &nodeprudata->policy) );
   assert(nodeprudata->policy->weights != NULL);
#else
   SCIP_CALL( SCIPkerasPolicyCreate(scip, &nodeprudata->policy) );
   assert(nodeprudata->polfname != NULL);
   SCIP_CALL( SCIPreadKerasPolicy(scip, nodeprudata->polfname, &nodeprudata->policy) );
   assert(nodeprudata->policy != NULL);
#endif

   /* Read problem specific features */
   nodeprudata->probfeats = NULL;
   if(strcmp(nodeprudata->probfeatsfname, DEFAULT_FILENAME) != 0)
   {
      FILE* featsfp = fopen(nodeprudata->probfeatsfname, "r");

      /* Assumes that the first line is the size of the feats */
      fscanf(featsfp, "%d\n", &(nodeprudata->probfeatsize));
      SCIP_ALLOC( BMSallocMemoryArray(&(nodeprudata->probfeats), nodeprudata->probfeatsize));

      /* Read the features */
      readProbFeats(featsfp, nodeprudata->probfeats, nodeprudata->probfeatsize);
      fclose(featsfp);
   }
   else{
      nodeprudata->probfeatsize=0;
   }

   /* create feat */
   nodeprudata->feat = NULL;
   if(strcmp(nodeprudata->normfname, DEFAULT_FILENAME) != 0){
      FILE* normF = fopen(nodeprudata->normfname, "r");
      SCIP_CALL( SCIPfeatCreate(scip, &nodeprudata->feat, normF, SCIP_FEATNODEPRU_SIZE + nodeprudata->probfeatsize, SCIP_FEATNODEPRU_SIZE) );
      fclose(normF);
   }
   else{
      SCIP_CALL( SCIPfeatCreate(scip, &nodeprudata->feat, NULL, SCIP_FEATNODEPRU_SIZE + nodeprudata->probfeatsize, SCIP_FEATNODEPRU_SIZE) );
   }
   assert(nodeprudata->feat != NULL);
   SCIPfeatSetMaxDepth(nodeprudata->feat, SCIPgetNBinVars(scip) + SCIPgetNIntVars(scip));

   nodeprudata->nprunes = 0;
   return SCIP_OKAY;
}

/** destructor of node pruner to free user data (called when SCIP is exiting) */
static
SCIP_DECL_NODEPRUEXIT(nodepruExitPolicy)
{
   SCIP_NODEPRUDATA* nodeprudata;
   assert(scip != NULL);
   assert(nodepru != NULL);

   nodeprudata = SCIPnodepruGetData(nodepru);

   assert(nodeprudata->feat != NULL);
   SCIP_CALL( SCIPfeatFree(scip, &nodeprudata->feat) );

#ifdef LIBLINEAR
   assert(nodeprudata->policy != NULL);
   SCIP_CALL( SCIPpolicyFree(scip, &nodeprudata->policy) );
#else
   assert(nodeprudata->policy != NULL);
   SCIP_CALL( SCIPkerasPolicyFree(scip, &nodeprudata->policy) );
#endif
 
   if(nodeprudata->probfeats != NULL)
      BMSfreeMemory(&(nodeprudata->probfeats));
 
   return SCIP_OKAY;
}

/** destructor of node pruner to free user data (called when SCIP is exiting) */
static
SCIP_DECL_NODEPRUFREE(nodepruFreePolicy)
{
   SCIP_NODEPRUDATA* nodeprudata;
   nodeprudata = SCIPnodepruGetData(nodepru);

   assert(nodeprudata != NULL);

   SCIPfreeBlockMemory(scip, &nodeprudata);

   SCIPnodepruSetData(nodepru, NULL);

   return SCIP_OKAY;
}

/** node pruning method of node pruner */
static
SCIP_DECL_NODEPRUPRUNE(nodepruPrunePolicy)
{
   SCIP_NODEPRUDATA* nodeprudata;
   SCIP_Real cutoff;

   assert(nodepru != NULL);
   assert(strcmp(SCIPnodepruGetName(nodepru), NODEPRU_NAME) == 0);
   assert(scip != NULL);
   assert(node != NULL);

   nodeprudata = SCIPnodepruGetData(nodepru);
   assert(nodeprudata != NULL);

   /* don't prune the root */
   if( SCIPnodeGetDepth(node) == 0 )
   {
      *prune = FALSE;
      return SCIP_OKAY;
   }
   else
   {
      SCIP_Real noise;
      SCIPcalcNodepruFeat(scip, node, nodeprudata->feat, \
                                    nodeprudata->probfeats, nodeprudata->probfeatsize);

      /*
      SCIPclockStart(nodeprudata->featcalctime, scip->set);
      SCIPclockStop(nodeprudata->featcalctime, scip->set);
      */

#ifdef LIBLINEAR
      SCIPcalcNodeScore(node, nodeprudata->feat, nodeprudata->policy, nodeprudata->probfeatsize);
      cutoff = 0;
#else
      SCIPcalcKerasNodeScore(node, nodeprudata->feat, &nodeprudata->policy, nodeprudata->probfeatsize);
      cutoff = 0.5;
#endif

      noise = getRandom(nodeprudata->score_noise);
      if( SCIPsetIsGT(scip->set, SCIPnodeGetScore(node) + noise, cutoff) )
      {
         *prune = TRUE;
         nodeprudata->nprunes++;
         SCIPdebugMessage("pruning node: #%"SCIP_LONGINT_FORMAT"\n", SCIPnodeGetNumber(node));
      }
      else
         *prune = FALSE;
   }

   return SCIP_OKAY;
}


/*
 * node pruner specific interface methods
 */

/** creates the uct node pruner and includes it in SCIP */
SCIP_RETCODE SCIPincludeNodepruPolicy(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_NODEPRUDATA* nodeprudata;
   SCIP_NODEPRU* nodepru;

   /* create policy node pruner data */
   SCIP_CALL( SCIPallocBlockMemory(scip, &nodeprudata) );

   nodepru = NULL;
   nodeprudata->polfname = NULL;
   nodeprudata->normfname = NULL;
   nodeprudata->probfeatsfname = NULL;

   /* use SCIPincludeNodepruBasic() plus setter functions if you want to set callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeNodepruBasic(scip, &nodepru, NODEPRU_NAME, NODEPRU_DESC, NODEPRU_STDPRIORITY,
          NODEPRU_MEMSAVEPRIORITY, nodepruPrunePolicy, nodeprudata) );

   assert(nodepru != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetNodepruCopy(scip, nodepru, NULL) );
   SCIP_CALL( SCIPsetNodepruInit(scip, nodepru, nodepruInitPolicy) );
   SCIP_CALL( SCIPsetNodepruExit(scip, nodepru, nodepruExitPolicy) );
   SCIP_CALL( SCIPsetNodepruFree(scip, nodepru, nodepruFreePolicy) );

   /* add policy node pruner parameters */
   SCIP_CALL( SCIPaddStringParam(scip, 
         "nodepruning/"NODEPRU_NAME"/polfname",
         "name of the policy model file",
         &nodeprudata->polfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip, 
         "nodepruning/"NODEPRU_NAME"/normfname",
         "name of the normalization file",
         &nodeprudata->normfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip, 
         "nodepruning/"NODEPRU_NAME"/score_noise",
         "Noise to be added to the score", 
         &nodeprudata->score_noise, FALSE, 0, 0, 1, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodepruning/"NODEPRU_NAME"/probfeatsfname",
         "name of the file with prob specific features",
         &nodeprudata->probfeatsfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );

   return SCIP_OKAY;
}
