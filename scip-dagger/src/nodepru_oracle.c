/**@file   nodepru_oracle.c
 * @brief  oracle node pruner which prunes all non-optimal nodes
 * @author He He
 *
 * the UCT node selection rule selects the next leaf according to a mixed score of the node's actual lower bound
 *
 * @note It should be avoided to switch to uct node selection after the branch and bound process has begun because
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/
#include <assert.h>
#include <string.h>
#include "nodepru_oracle.h"
#include "nodesel_oracle.h"
#include "scip/sol.h"
#include "scip/struct_set.h"
#include "scip/tree.h"
#include "feat.h"

#define NODEPRU_NAME            "oracle"
#define NODEPRU_DESC            "node pruner which always prunes non-optimal nodes"
#define NODEPRU_STDPRIORITY     10
#define NODEPRU_MEMSAVEPRIORITY 0

#define DEFAULT_FILENAME        ""

/*
 * Data structures
 */

/** node pruner data */
struct SCIP_NodepruData
{
   SCIP_SOL*          optsol;             /**< optimal solution */
   SCIP_FEAT*         feat;               /**< optimal solution */
   char*              solfname;           /**< name of the solution file */
   char*              trjfname;           /**< name of the trajectory file */
   SCIP_Bool          checkopt;           /**< need to check node optimality? (don't need to if node selector is oracle or dagger */
   FILE*              wfile;
   FILE*              trjfile;

   /* Problem related features */
   char*              probfeatsfname;
   SCIP_Real*         probfeats;
   int                probfeatsize;
   SCIP_Real          scale;
   SCIP_Real          margin;
   SCIP_Real          depth;
};

/*
 * Callback methods of node pruner
 */


/** solving process initialization method of node pruner (called when branch and bound process is about to begin) */
static
SCIP_DECL_NODEPRUINIT(nodepruInitOracle)
{
   SCIP_NODEPRUDATA* nodeprudata;
   assert(scip != NULL);
   assert(nodepru != NULL);

   nodeprudata = SCIPnodepruGetData(nodepru);

   assert(nodeprudata != NULL);
   /** solfname should be set before including nodepruoracle */
   assert(nodeprudata->solfname != NULL);
   nodeprudata->optsol = NULL;

   SCIP_CALL( SCIPreadOptSol(scip, nodeprudata->solfname, &nodeprudata->optsol) );
   assert(nodeprudata->optsol != NULL);
#ifdef SCIP_DEBUG
   SCIP_CALL( SCIPprintSol(scip, nodeprudata->optsol, NULL, FALSE) );
#endif

   if( strcmp(SCIPnodeselGetName(SCIPgetNodesel(scip)), "oracle") == 0 ||
       strcmp(SCIPnodeselGetName(SCIPgetNodesel(scip)), "dagger") == 0 )
      nodeprudata->checkopt = FALSE;
   else
      nodeprudata->checkopt = TRUE;

   /* Trajectory file for oracle or expert */
   nodeprudata->trjfile = NULL;
   if( nodeprudata->trjfname != NULL )
   {
      char wfname[SCIP_MAXSTRLEN];
      strcpy(wfname, nodeprudata->trjfname);
      strcat(wfname, ".weight");
      nodeprudata->wfile = fopen(wfname, "a");
      nodeprudata->trjfile = fopen(nodeprudata->trjfname, "a");
   }

   /* Read problem specific features */
   nodeprudata->probfeats = NULL;
   if(strcmp(nodeprudata->probfeatsfname, DEFAULT_FILENAME) != 0)
   {
      FILE* featsfp = fopen(nodeprudata->probfeatsfname, "r");
      assert(featsfp != NULL);

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
   SCIP_CALL( SCIPfeatCreate(scip, &nodeprudata->feat, NULL, SCIP_FEATNODEPRU_SIZE + nodeprudata->probfeatsize, SCIP_FEATNODEPRU_SIZE) );
   assert(nodeprudata->feat != NULL);
   SCIPfeatSetMaxDepth(nodeprudata->feat, SCIPgetNBinVars(scip) + SCIPgetNIntVars(scip));

   return SCIP_OKAY;
}

/** deinitialization method of node selector (called before transformed problem is freed) */
static
SCIP_DECL_NODEPRUEXIT(nodepruExitOracle)
{
   SCIP_NODEPRUDATA* nodeprudata;
   assert(scip != NULL);
   assert(nodepru != NULL);

   nodeprudata = SCIPnodepruGetData(nodepru);

   assert(nodeprudata->optsol != NULL);
   SCIP_CALL( SCIPfreeSolSelf(scip, &nodeprudata->optsol) );
   nodeprudata->optsol = NULL;

   if( nodeprudata->feat != NULL )
   {
      SCIP_CALL( SCIPfeatFree(scip, &nodeprudata->feat) );
      nodeprudata->feat = NULL;
   }

   if( nodeprudata->trjfile != NULL)
   {
      fclose(nodeprudata->wfile);
      fclose(nodeprudata->trjfile);
      nodeprudata->trjfile = NULL;
      nodeprudata->wfile = NULL;
   }

   if(nodeprudata->probfeats != NULL)
      BMSfreeMemory(&(nodeprudata->probfeats));

   nodeprudata->checkopt = FALSE;
   return SCIP_OKAY;
}

/** destructor of node pruner to free user data (called when SCIP is exiting) */
static
SCIP_DECL_NODEPRUFREE(nodepruFreeOracle)
{
   SCIP_NODEPRUDATA* nodeprudata;
   nodeprudata = SCIPnodepruGetData(nodepru);

   assert(nodeprudata != NULL);

   SCIPfreeBlockMemory(scip, &nodeprudata);

   SCIPnodepruSetData(nodepru, NULL);

   return SCIP_OKAY;
}

SCIP_Bool checkDepth(SCIP_NODE* node, SCIP_Real depth)
{
   SCIP_NODE* cnode=node;   
   while (depth > 0){
      depth = depth-1;
      cnode = SCIPnodeGetParent(cnode);
      if(SCIPnodeGetDepth(cnode) == 0)
            break;
   }
   return SCIPnodeIsOptimal(cnode);
}

/** node pruning method of node pruner */
static
SCIP_DECL_NODEPRUPRUNE(nodepruPruneOracle)
{
   SCIP_NODEPRUDATA* nodeprudata;
   SCIP_SOL* optsol;
   SCIP_Bool isoptimal;

   assert(nodepru != NULL);
   assert(strcmp(SCIPnodepruGetName(nodepru), NODEPRU_NAME) == 0);
   assert(scip != NULL);
   assert(node != NULL);

   nodeprudata = SCIPnodepruGetData(nodepru);
   assert(nodeprudata != NULL);
   optsol = nodeprudata->optsol;
   assert(optsol != NULL);

   // Allocate memory for sol flag
   SCIPnodeAllocSolFlag(node, 1);

   /* don't prune the root */
   if( SCIPnodeGetDepth(node) == 0 )
   {
      *prune = FALSE;
      return SCIP_OKAY;
   }
   else
   {
      if( nodeprudata->checkopt ){
         SCIPnodeCheckOptimal(scip, node, optsol, 0, 0, nodeprudata->margin);
      }
      isoptimal = SCIPnodeIsOptimal(node);
      if (isoptimal != MY_UNKNOWN){
        if( isoptimal )
        {
           SCIPnodeSetSolIdx(node, 0);
           *prune = FALSE;
        }
        else
        {
           SCIP_Bool opt = checkDepth(node, nodeprudata->depth);
           SCIPdebugMessage("pruning node: #%"SCIP_LONGINT_FORMAT"\n", SCIPnodeGetNumber(node));
           if (opt)
              *prune = FALSE;
           else
              *prune = TRUE;
        }
      }
      else{
        /* Don't prune if the solution is unknown -- fair enough right ? */
        *prune = FALSE;
      }

#ifndef SCIP_DEBUG
      if( nodeprudata->trjfile != NULL )
      {
#endif
         SCIPcalcNodepruFeat(scip, node, nodeprudata->feat, \
                                    nodeprudata->probfeats, nodeprudata->probfeatsize);

         SCIPdebugMessage("node pruning feature of node #%"SCIP_LONGINT_FORMAT"\n", SCIPnodeGetNumber(node));
#ifdef LIBLINEAR
         SCIPfeatLIBSVMPrint(scip, nodeprudata->trjfile, nodeprudata->wfile, \
                                    nodeprudata->feat, nodeprudata->scale, isoptimal ? -1 : 1);
#else
         SCIPfeatLIBSVMPrint(scip, nodeprudata->trjfile, nodeprudata->wfile, \
                                    nodeprudata->feat, nodeprudata->scale, isoptimal ? 0 : 1);
#endif
      }
#ifndef SCIP_DEBUG
   }
#endif

   return SCIP_OKAY;
}

/*
 * node pruner specific interface methods
 */

/** creates the uct node pruner and includes it in SCIP */
SCIP_RETCODE SCIPincludeNodepruOracle(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_NODEPRUDATA* nodeprudata;
   SCIP_NODEPRU* nodepru;

   /* create oracle node pruner data */
   SCIP_CALL( SCIPallocBlockMemory(scip, &nodeprudata) );

   nodepru = NULL;
   nodeprudata->optsol = NULL;
   nodeprudata->solfname = NULL;
   nodeprudata->trjfname = NULL;
   nodeprudata->probfeatsfname = NULL;
   nodeprudata->scale = 1;

   /* use SCIPincludeNodepruBasic() plus setter functions if you want to set callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeNodepruBasic(scip, &nodepru, NODEPRU_NAME, NODEPRU_DESC, NODEPRU_STDPRIORITY,
          NODEPRU_MEMSAVEPRIORITY, nodepruPruneOracle, nodeprudata) );

   assert(nodepru != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetNodepruCopy(scip, nodepru, NULL) );
   SCIP_CALL( SCIPsetNodepruInit(scip, nodepru, nodepruInitOracle) );
   SCIP_CALL( SCIPsetNodepruExit(scip, nodepru, nodepruExitOracle) );
   SCIP_CALL( SCIPsetNodepruFree(scip, nodepru, nodepruFreeOracle) );

   /* add oracle node pruner parameters */
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodepruning/"NODEPRU_NAME"/solfname",
         "name of the optimal solution file",
         &nodeprudata->solfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodepruning/"NODEPRU_NAME"/trjfname",
         "name of the file to write node pruning trajectories",
         &nodeprudata->trjfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodepruning/"NODEPRU_NAME"/probfeatsfname",
         "name of the file with prob specific features",
         &nodeprudata->probfeatsfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip,
         "nodepruning/"NODEPRU_NAME"/scale",
         "Scale the weights of the data points",
         &nodeprudata->scale, FALSE, 1, 0.001, SCIP_REAL_MAX, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip,
         "nodepruning/"NODEPRU_NAME"/margin",
         "Margin around optimal solution",
         &nodeprudata->margin, FALSE, 0, 0, 0.05, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip, 
         "nodepruning/"NODEPRU_NAME"/depth",
         "Depth of exploration", 
         &nodeprudata->depth, FALSE, 2, 0, SCIP_REAL_MAX, NULL, NULL) );

   return SCIP_OKAY;
}
