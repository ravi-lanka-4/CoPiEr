/**@file   nodepru_dagger.c
 * @brief  node pruner which prunes node according to a learned policy and writes examples according to the oracle choice
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
#include <unistd.h>
#include "nodepru_dagger.h"
#include "nodepru_oracle.h"
#include "nodesel_oracle.h"
#include "helper.h"
#include "feat.h"
#include "policy.h"
#include "unistd.h"
#include "struct_policy.h"
#include "scip/def.h"
#include "scip/sol.h"
#include "scip/tree.h"
#include "scip/set.h"
#include "scip/struct_set.h"
#include "scip/struct_scip.h"
#include "scip/struct_stat.h"

#define NODEPRU_NAME            "dagger"
#define NODEPRU_DESC            "node pruner which pruects node according to a policy but writes exampels according to the oracle"
#define NODEPRU_STDPRIORITY     10
#define NODEPRU_MEMSAVEPRIORITY 0

#define DEFAULT_FILENAME        ""

extern float getRandom(float stddev);
static OPT_FLAG checkOptimal(SCIP* scip, SCIP_NODE* node, SCIP_NODEPRUDATA* nodeprudata, SCIP_Bool useSol);

/*
 * Data structures
 */

/** node pruner data */
struct SCIP_NodepruData
{
   char*              solfname;           /**< name of the solution file */
   char*              polfname;           /**< name of the solution file */
#ifdef LIBLINEAR
   SCIP_POLICY*       policy;
#else
   void*              policy;
#endif
   char*              trjfname;           /**< name of the trajectory file */
   char*              normfname;
   FILE*              wfile;
   FILE*              trjfile;
   SCIP_FEAT*         feat;
   SCIP_Bool          checkopt;           /**< need to check node optimality? (don't need to if node selector is oracle or dagger */
   int                nprunes;            /**< number of nodes pruned */
   int                nnodes;             /**< number of nodes checked */
   int                nfalsepos;          /**< number of optimal nodes pruned */
   int                nfalseneg;          /**< number of non-optimal nodes not pruned */
   unsigned int       randseed;
   SCIP_Real          scale;
   SCIP_Real          margin;

   /* Optimal traces */
   SCIP_Real          ogapThreshold;      /**< Treshold to consider the solutions */
   int                isTrace;
   SCIP_Longint       nfeasiblesols;
   SCIP_SOL**         feasiblesols;             /**< optimal solution */
   SCIP_Bool*         solflag;
   SCIP_Real          score_noise;        /**< Score noise */

   /* Problem related features */
   char*              probfeatsfname;
   SCIP_Real*         probfeats;
   int                probfeatsize;

   /* Handle partial solution */
   SCIP_Bool          useSols;
};

void SCIPnodeprudaggerPrintStatistics(
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
   printf("Node pruner      :\n");
   printf("Nodes pruned     : %d/%d\n", nodeprudata->nprunes, nodeprudata->nnodes);
   printf("FP pruned        : %d/%d\n", nodeprudata->nfalsepos, nodeprudata->nnodes);
   printf("FN pruned        : %d/%d\n", nodeprudata->nfalseneg, nodeprudata->nnodes);
   printf("Pruning time     : %10.2f\n", SCIPnodepruGetTime(nodepru));

//   SCIPmessageFPrintInfo(scip->messagehdlr, file,
//         "Node pruner        :\n");
//   SCIPmessageFPrintInfo(scip->messagehdlr, file,
//         "  nodes pruned     : %d/%d\n", nodeprudata->nprunes, nodeprudata->nnodes);
//   SCIPmessageFPrintInfo(scip->messagehdlr, file,
//         "  FP pruned        : %d/%d\n", nodeprudata->nfalsepos, nodeprudata->nnodes);
//   SCIPmessageFPrintInfo(scip->messagehdlr, file,
//         "  FN pruned        : %d/%d\n", nodeprudata->nfalseneg, nodeprudata->nnodes);
//   SCIPmessageFPrintInfo(scip->messagehdlr, file,
//         "  pruning time     : %10.2f\n", SCIPnodepruGetTime(nodepru));
}

/** solving process initialization method of node pruner (called when branch and bound process is about to begin) */
static
SCIP_DECL_NODEPRUINIT(nodepruInitDagger)
{
   SCIP_NODEPRUDATA* nodeprudata;
   SCIP_Real bestobj;
   int i=0;
   assert(scip != NULL);
   assert(nodepru != NULL);

   nodeprudata = SCIPnodepruGetData(nodepru);
   assert(nodeprudata != NULL);

   /* solfname should be set before including nodeprudagger */
   nodeprudata->isTrace = (scip->set->allsols_prefix[0] != '-');
   nodeprudata->nfeasiblesols = 0;
   if(!(nodeprudata->isTrace))
   {
      // No Trace provided; only solution file needed in this case
      assert(nodeprudata->solfname != NULL);
      nodeprudata->nfeasiblesols = 1;

      // Assign memory for the sols
      BMSallocMemoryArray(&(nodeprudata->feasiblesols), nodeprudata->nfeasiblesols);

      nodeprudata->feasiblesols[0] = NULL;
      SCIP_CALL( SCIPreadOptSol(scip, nodeprudata->solfname, &(nodeprudata->feasiblesols[0])));
      assert(nodeprudata->feasiblesols[0] != NULL);
   }
   else{
      int readcount=0;
      nodeprudata->nfeasiblesols = countnFeasible(scip->set->allsols_prefix);
      if (nodeprudata->solfname != NULL)
         nodeprudata->nfeasiblesols = nodeprudata->nfeasiblesols + 1;

      // Assign memory for the sols
      BMSallocMemoryArray(&(nodeprudata->feasiblesols), nodeprudata->nfeasiblesols);
      if (nodeprudata->solfname != NULL){
         nodeprudata->feasiblesols[0] = NULL;
         SCIP_CALL( SCIPreadOptSol(scip, nodeprudata->solfname, &(nodeprudata->feasiblesols[0]) ));
         assert(nodeprudata->feasiblesols[0] != NULL);
         readcount = readcount + 1;
      }

      // Read all the trace related solution
      for(i=readcount; i<nodeprudata->nfeasiblesols; i++){
         char solfname[SCIP_MAXSTRLEN];
         getSolFname(solfname, scip->set->allsols_prefix, i-readcount);

         nodeprudata->feasiblesols[i] = NULL;
         SCIP_CALL( SCIPreadOptSol(scip, solfname, &(nodeprudata->feasiblesols[i]) ));
         assert(nodeprudata->feasiblesols[i] != NULL);
      }
   }

   // Get best objective value from all the solutions 
   bestobj = getBestObj(nodeprudata->feasiblesols, nodeprudata->nfeasiblesols);

   /* Create the flag for using the solution based on the threshold */
   BMSallocMemoryArray(&(nodeprudata->solflag), nodeprudata->nfeasiblesols);
   for(i=0; i<nodeprudata->nfeasiblesols; i++){
      SCIP_Real ogap;
      SCIP_Real cobj = SCIPsolGetOrigObj(nodeprudata->feasiblesols[i]);
      ogap = fabs((cobj - bestobj)/bestobj);
      if (!nodeprudata->useSols){
        if (ogap <= nodeprudata->ogapThreshold)
          nodeprudata->solflag[i] = TRUE;
        else
          nodeprudata->solflag[i] = FALSE;
      }
      else{
        nodeprudata->solflag[i] = TRUE;
      }
   }

#ifdef SCIP_DEBUG
   SCIP_CALL( SCIPprintSol(scip, nodeprudata->feasiblesols[0], NULL, FALSE) );
#endif

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

   /* open trajectory file for writing */
   /* open in appending mode for writing training file from multiple problems */
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

   if( strcmp(SCIPnodeselGetName(SCIPgetNodesel(scip)), "oracle") == 0 ||
       strcmp(SCIPnodeselGetName(SCIPgetNodesel(scip)), "dagger") == 0 )
      nodeprudata->checkopt = FALSE;
   else
      nodeprudata->checkopt = TRUE;

   nodeprudata->nprunes   = 0;
   nodeprudata->nnodes    = 0;
   nodeprudata->nfalsepos = 0;
   nodeprudata->nfalseneg = 0;
   nodeprudata->randseed  = 0;

   return SCIP_OKAY;
}

/** destructor of node pruner to free user data (called when SCIP is exiting) */
static
SCIP_DECL_NODEPRUEXIT(nodepruExitDagger)
{
   int i;
   SCIP_NODEPRUDATA* nodeprudata;
   assert(scip != NULL);
   assert(nodepru != NULL);

   nodeprudata = SCIPnodepruGetData(nodepru);

   assert(nodeprudata->feasiblesols != NULL);
   for(i=0; i<nodeprudata->nfeasiblesols; i++){
      assert(nodeprudata->feasiblesols[i] != NULL);
      SCIP_CALL( SCIPfreeSolSelf(scip, &(nodeprudata->feasiblesols[i])) );
   }
   BMSfreeMemory(&(nodeprudata->feasiblesols));
   BMSfreeMemory(&(nodeprudata->solflag));

   if( nodeprudata->trjfile != NULL)
   {
      fclose(nodeprudata->wfile);
      fclose(nodeprudata->trjfile);
   }

   if(nodeprudata->probfeats != NULL)
      BMSfreeMemory(&(nodeprudata->probfeats));

   assert(nodeprudata->feat != NULL);
   SCIP_CALL( SCIPfeatFree(scip, &nodeprudata->feat) );

#ifdef LIBLINEAR
   assert(nodeprudata->policy != NULL);
   SCIP_CALL( SCIPpolicyFree(scip, &nodeprudata->policy) );
#else
   assert(nodeprudata->policy != NULL);
   SCIP_CALL( SCIPkerasPolicyFree(scip, &nodeprudata->policy) );
#endif

   return SCIP_OKAY;
}

/** destructor of node pruner to free user data (called when SCIP is exiting) */
static
SCIP_DECL_NODEPRUFREE(nodepruFreeDagger)
{
   SCIP_NODEPRUDATA* nodeprudata;
   nodeprudata = SCIPnodepruGetData(nodepru);

   assert(nodeprudata != NULL);
   SCIPfreeBlockMemory(scip, &nodeprudata);
   SCIPnodepruSetData(nodepru, NULL);
   return SCIP_OKAY;
}

static 
OPT_FLAG checkOptimal(SCIP* scip, SCIP_NODE* node, SCIP_NODEPRUDATA* nodeprudata, SCIP_Bool useSol){
   int i=0;
   OPT_FLAG isoptimal; 
   for(i=0; i<nodeprudata->nfeasiblesols; i++){
      /* Condition for using the solution file */            
      if(nodeprudata->solflag[i]){
          SCIPnodeCheckOptimal(scip, node, nodeprudata->feasiblesols[i], i, useSol, nodeprudata->margin);
      }
   }
   isoptimal = SCIPnodeIsOptimal(node);
   return isoptimal;
}

/** node pruning method of node pruner */
static
SCIP_DECL_NODEPRUPRUNE(nodepruPruneDagger)
{
   SCIP_NODEPRUDATA* nodeprudata;
   OPT_FLAG isoptimal=FALSE;
   int scale;
   float rand;
   SCIP_Real cutoff;
   SCIP_Bool writedata=FALSE;

   assert(nodepru != NULL);
   assert(strcmp(SCIPnodepruGetName(nodepru), NODEPRU_NAME) == 0);
   assert(scip != NULL);
   assert(node != NULL);

   nodeprudata = SCIPnodepruGetData(nodepru);
   assert(nodeprudata != NULL);

   // Allocate memory for sol flag
   SCIPnodeAllocSolFlag(node, nodeprudata->nfeasiblesols);

   /* don't prune the root */
   if( SCIPnodeGetDepth(node) == 0 )
   {
      *prune = FALSE;
      return SCIP_OKAY;
   }
   else if(nodeprudata->useSols){
      isoptimal = checkOptimal(scip, node, nodeprudata, TRUE);
      if (isoptimal != MY_UNKNOWN){
        /* If solution is known then use it; Otherwise use the policy to make the decision */
        if( isoptimal == TRUE)
            *prune = FALSE;
        else
            *prune = TRUE;
        return SCIP_OKAY;
      }
   }

   {
      /*
      SCIP_Real rand; */
      SCIP_Real noise;

      SCIPcalcNodepruFeat(scip, node, nodeprudata->feat, \
                                    nodeprudata->probfeats, nodeprudata->probfeatsize);

#ifdef LIBLINEAR
      SCIPcalcNodeScore(node, nodeprudata->feat, nodeprudata->policy, nodeprudata->probfeatsize);
      cutoff = 0;
#else
      SCIPcalcKerasNodeScore(node, nodeprudata->feat, &nodeprudata->policy, nodeprudata->probfeatsize);
      cutoff = 0.5;
#endif

      if( nodeprudata->checkopt ){
        // Check for optimality in all the feasible 
        // solution and traces through only optimal solution
        isoptimal = checkOptimal(scip, node, nodeprudata, FALSE);
      }
      isoptimal = SCIPnodeIsOptimal(node);
      noise = getRandom(nodeprudata->score_noise);

      rand = SCIPgetRandomReal(0.0, 1.0, &nodeprudata->randseed);
      /*if( rand < 0.2 )
         *prune = (isoptimal == FALSE);
      else*/
      { 
         if( SCIPsetIsGT(scip->set, SCIPnodeGetScore(node) + noise, cutoff) )
         {
            /* don't prune optimal */
            /*if( isoptimal )
               *prune = FALSE;
            else*/
            { 
               *prune = TRUE;
               SCIPdebugMessage("pruning node: #%"SCIP_LONGINT_FORMAT"\n", SCIPnodeGetNumber(node));
               nodeprudata->nprunes++;
            } 
         }
         else
            *prune = FALSE;
      }
      nodeprudata->nnodes++;
      //scale = 1;
      if( isoptimal && *prune ){
         nodeprudata->nfalsepos++;
         writedata = TRUE;
         //scale = 1;
      }
      else if( (!isoptimal) && (!*prune) ){
         nodeprudata->nfalseneg++;
         writedata = TRUE;
         //scale = 1;
      }

/* write feature vector to stdout in debug mode */
#ifndef SCIP_DEBUG
      /* write examples */
      if(nodeprudata->trjfile != NULL)
      {
#endif
         SCIPdebugMessage("node pruning feature of node #%"SCIP_LONGINT_FORMAT"\n", SCIPnodeGetNumber(node));
#ifdef LIBLINEAR
         SCIPfeatLIBSVMPrint(scip, nodeprudata->trjfile, nodeprudata->wfile, \
                                    nodeprudata->feat, nodeprudata->scale, isoptimal ? -1 : 1);
#else
         SCIPfeatLIBSVMPrint(scip, nodeprudata->trjfile, nodeprudata->wfile, \
                                    nodeprudata->feat, nodeprudata->scale, isoptimal ? 0 : 1);
#endif
#ifndef SCIP_DEBUG
      }
#endif
   }

   return SCIP_OKAY;
}

/*
 * node pruner specific interface methods
 */

/** creates the uct node pruner and includes it in SCIP */
SCIP_RETCODE SCIPincludeNodepruDagger(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_NODEPRUDATA* nodeprudata;
   SCIP_NODEPRU* nodepru;

   /* create dagger node pruner data */
   SCIP_CALL( SCIPallocBlockMemory(scip, &nodeprudata) );

   nodepru = NULL;
   nodeprudata->feasiblesols = NULL;
   nodeprudata->solfname = NULL;
   nodeprudata->trjfname = NULL;
   nodeprudata->normfname = NULL;
   nodeprudata->polfname = NULL;
   nodeprudata->probfeatsfname = NULL;
   nodeprudata->scale = 1;

   /* use SCIPincludeNodepruBasic() plus setter functions if you want to set callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeNodepruBasic(scip, &nodepru, NODEPRU_NAME, NODEPRU_DESC, NODEPRU_STDPRIORITY,
          NODEPRU_MEMSAVEPRIORITY, nodepruPruneDagger, nodeprudata) );

   assert(nodepru != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetNodepruCopy(scip, nodepru, NULL) );
   SCIP_CALL( SCIPsetNodepruInit(scip, nodepru, nodepruInitDagger) );
   SCIP_CALL( SCIPsetNodepruExit(scip, nodepru, nodepruExitDagger) );
   SCIP_CALL( SCIPsetNodepruFree(scip, nodepru, nodepruFreeDagger) );

   /* add dagger node pruner parameters */
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodepruning/"NODEPRU_NAME"/solfname",
         "name of the optimal solution file",
         &nodeprudata->solfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodepruning/"NODEPRU_NAME"/normfname",
         "name of the normalization file",
         &nodeprudata->normfname , FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodepruning/"NODEPRU_NAME"/trjfname",
         "name of the file to write node pruning trajectories",
         &nodeprudata->trjfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodepruning/"NODEPRU_NAME"/polfname",
         "name of the policy model file",
         &nodeprudata->polfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodepruning/"NODEPRU_NAME"/probfeatsfname",
         "name of the file with prob specific features",
         &nodeprudata->probfeatsfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip, 
         "nodepruning/"NODEPRU_NAME"/ogapThreshold",
         "optimality gap threshold of solutions to consider", 
         &nodeprudata->ogapThreshold, FALSE, 0, 0, 1, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip, 
         "nodepruning/"NODEPRU_NAME"/score_noise",
         "Noise to be added to the score", 
         &nodeprudata->score_noise, FALSE, 0, 0, 1, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip,
         "nodepruning/"NODEPRU_NAME"/scale",
         "Scale the weights of the data points",
         &nodeprudata->scale, FALSE, 1, 0.001, SCIP_REAL_MAX, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip,
         "nodepruning/"NODEPRU_NAME"/margin",
         "Margin around optimal solution",
         &nodeprudata->margin, FALSE, 0, 0, 0.05, NULL, NULL) );
   SCIP_CALL( SCIPaddBoolParam(scip, 
         "nodepruning/"NODEPRU_NAME"/useSols",
         "Use the solutions -- for spatial policy data aggregation", 
         &nodeprudata->useSols, FALSE, FALSE, NULL, NULL) );

   return SCIP_OKAY;
}

