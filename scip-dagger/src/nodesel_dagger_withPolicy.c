/**@file   nodesel_dagger.c
 * @brief  uct node selector which balances exploration and exploitation by considering node visits
 * @author Gregor Hendel
 *
 * the UCT node selection rule selects the next leaf according to a mixed score of the node's actual lower bound
 *
 * The idea of UCT node selection for MIP appeared in:
 *
 * The authors adapted a game-tree exploration scheme called UCB to MIP trees. Starting from the root node as current node,
 *
 * The node selector features several parameters:
 *
 * @note It should be avoided to switch to uct node selection after the branch and bound process has begun because
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/
#include <assert.h>
#include <string.h>
#include "nodesel_dagger.h"
#include "nodesel_oracle.h"
#include "helper.h"
#include "feat.h"
#include "policy.h"
#include "helper.h"
#include "struct_policy.h"
#include "scip/def.h"
#include "scip/sol.h"
#include "scip/tree.h"
#include "scip/stat.h"
#include "scip/struct_set.h"
#include "scip/struct_scip.h"
#include "scip/pub_tree.h"
#include "struct_nodesel_dagger.h"

#define NODESEL_NAME            "dagger"
#define NODESEL_DESC            "node selector which selects node according to a policy but writes exampels according to the oracle"
#define NODESEL_STDPRIORITY     10
#define NODESEL_MEMSAVEPRIORITY 0

#define DEFAULT_FILENAME        ""

extern float getRandom(float stddev);

static
SCIP_RETCODE createFeatDiff(
   SCIP*                scip,
   SCIP_NODE*           node,
   SCIP_NODESELDATA*    nodeseldata
);

/*
 * Data structures
 */

void SCIPnodeseldaggerPrintStatistics(
   SCIP*                 scip,
   SCIP_NODESEL*         nodesel,
   FILE*                 file
   )
{
   SCIP_NODESELDATA* nodeseldata;

   assert(scip != NULL);
   assert(nodesel != NULL);

   nodeseldata = SCIPnodeselGetData(nodesel);
   assert(nodeseldata != NULL);

   printf("Node selector      :\n");
   printf("Comp error rate  : %d/%d\n", nodeseldata->nerrors, nodeseldata->ncomps);
   printf("selection time   : %10.2f\n", SCIPnodeselGetTime(nodesel));

//   SCIPmessageFPrintInfo(scip->messagehdlr, file,
//         "Node selector      :\n");
//   SCIPmessageFPrintInfo(scip->messagehdlr, file,
//         "  comp error rate  : %d/%d\n", nodeseldata->nerrors, nodeseldata->ncomps);
//   SCIPmessageFPrintInfo(scip->messagehdlr, file,
//         "  selection time   : %10.2f\n", SCIPnodeselGetTime(nodesel));
}

/** solving process initialization method of node selector (called when branch and bound process is about to begin) */
static
SCIP_DECL_NODESELINIT(nodeselInitDagger)
{
   SCIP_NODESELDATA* nodeseldata;
   SCIP_Real bestobj;
   int i=0;
   FILE* normF=NULL;
   assert(scip != NULL);
   assert(nodesel != NULL);

   nodeseldata = SCIPnodeselGetData(nodesel);
   normF = fopen(nodeseldata->normfname, "r");
   assert(nodeseldata != NULL);

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

   /* solfname should be set before including nodeprudagger */
   nodeseldata->isTrace = (scip->set->allsols_prefix[0] != '-');
   nodeseldata->nfeasiblesols = 0;
   if(!(nodeseldata->isTrace))
   {
      // No Trace provided; only solution file needed in this case
      assert(nodeseldata->solfname != NULL);
      nodeseldata->nfeasiblesols = 1;

      // Assign memory for the sols
      BMSallocMemoryArray(&(nodeseldata->feasiblesols), nodeseldata->nfeasiblesols);

      nodeseldata->feasiblesols[0] = NULL;
      SCIP_CALL( SCIPreadOptSol(scip, nodeseldata->solfname, &(nodeseldata->feasiblesols[0])));
      assert(nodeseldata->feasiblesols[0] != NULL);
   }
   else{
      int readcount=0;
      nodeseldata->nfeasiblesols = countnFeasible(scip->set->allsols_prefix);
      if (nodeseldata->solfname != NULL)
         nodeseldata->nfeasiblesols = nodeseldata->nfeasiblesols + 1;

      // Assign memory for the sols
      BMSallocMemoryArray(&(nodeseldata->feasiblesols), nodeseldata->nfeasiblesols);
      if (nodeseldata->solfname != NULL){
         nodeseldata->feasiblesols[0] = NULL;
         SCIP_CALL( SCIPreadOptSol(scip, nodeseldata->solfname, &(nodeseldata->feasiblesols[0]) ));
         assert(nodeseldata->feasiblesols[0] != NULL);
         readcount = readcount + 1;
      }

      // Read all the trace related solution
      for(i=readcount; i<nodeseldata->nfeasiblesols; i++){
         char solfname[SCIP_MAXSTRLEN];
         getSolFname(solfname, scip->set->allsols_prefix, i-readcount);

         nodeseldata->feasiblesols[i] = NULL;
         SCIP_CALL( SCIPreadOptSol(scip, solfname, &(nodeseldata->feasiblesols[i]) ));
         assert(nodeseldata->feasiblesols[i] != NULL);
      }
   }

   // Get best objective value from all the solutions 
   bestobj = getBestObj(nodeseldata->feasiblesols, nodeseldata->nfeasiblesols);

   /* Create the flag for using the solution based on the threshold */
   BMSallocMemoryArray(&(nodeseldata->solflag), nodeseldata->nfeasiblesols);
   BMSallocMemoryArray(&(nodeseldata->feasiblescore), nodeseldata->nfeasiblesols);
   for(i=0; i<nodeseldata->nfeasiblesols; i++){
      SCIP_Real ogap;
      SCIP_Real cobj = SCIPsolGetOrigObj(nodeseldata->feasiblesols[i]);
      ogap = fabs((cobj - bestobj)/(0.01+bestobj));
      nodeseldata->feasiblescore[i] = SCIP_LONGINT_MIN;
      if (ogap <= nodeseldata->ogapThreshold)
         nodeseldata->solflag[i] = TRUE;
      else
         nodeseldata->solflag[i] = FALSE;
   }

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

   /* open trajectory file for writing */
   /* open in appending mode for writing training file from multiple problems */
   nodeseldata->trjfile1 = NULL;
   nodeseldata->trjfile2 = NULL;
   if( nodeseldata->trjfname != NULL )
   {
      char wfname[SCIP_MAXSTRLEN];
      strcpy(wfname, nodeseldata->trjfname);
      strcat(wfname, ".weight");
      nodeseldata->wfile = fopen(wfname, "a");

#ifdef LIBLINEAR
      nodeseldata->trjfile1 = fopen(nodeseldata->trjfname, "a");
#else
      strcpy(wfname, nodeseldata->trjfname);
      strcat(wfname, ".1");
      nodeseldata->trjfile1 = fopen(wfname, "a");

      strcpy(wfname, nodeseldata->trjfname);
      strcat(wfname, ".2");
      nodeseldata->trjfile2 = fopen(wfname, "a");
#endif
   }

   /* create feat */
   nodeseldata->feat = NULL;
   if(strcmp(nodeseldata->normfname, DEFAULT_FILENAME) != 0){
      SCIP_CALL( SCIPfeatCreate(scip, &nodeseldata->feat, normF, 
                                 SCIP_FEATNODESEL_SIZE + nodeseldata->probfeatsize + BUFF, 
                                 SCIP_FEATNODESEL_SIZE) );
      fclose(normF);
   }
   else{
      SCIP_CALL( SCIPfeatCreate(scip, &nodeseldata->feat, NULL, 
                                 SCIP_FEATNODESEL_SIZE + nodeseldata->probfeatsize + BUFF, 
                                 SCIP_FEATNODESEL_SIZE) );
   }
   assert(nodeseldata->feat != NULL);
   SCIPfeatSetMaxDepth(nodeseldata->feat, SCIPgetNBinVars(scip) + SCIPgetNIntVars(scip));

   /* create optimal node feat */
   nodeseldata->feasiblefeat = NULL;
   BMSallocMemoryArray(&(nodeseldata->feasiblefeat), nodeseldata->nfeasiblesols);
   for(i=0; i<nodeseldata->nfeasiblesols; i++){
      if(strcmp(nodeseldata->normfname, DEFAULT_FILENAME) != 0){
            FILE* normF = fopen(nodeseldata->normfname, "r");
            SCIP_CALL( SCIPfeatCreate(scip, &(nodeseldata->feasiblefeat[i]), normF, 
                                       SCIP_FEATNODESEL_SIZE + nodeseldata->probfeatsize + BUFF, 
                                       SCIP_FEATNODESEL_SIZE) );
            fclose(normF);
      }
      else{
            SCIP_CALL( SCIPfeatCreate(scip, &(nodeseldata->feasiblefeat[i]), NULL, 
                                       SCIP_FEATNODESEL_SIZE + nodeseldata->probfeatsize + BUFF, 
                                       SCIP_FEATNODESEL_SIZE) );
      }
      assert(&(nodeseldata->feasiblefeat[i]) != NULL);
      SCIPfeatSetMaxDepth(nodeseldata->feasiblefeat[i], SCIPgetNBinVars(scip) + SCIPgetNIntVars(scip));
   }

   nodeseldata->optnodenumber = -1;
   nodeseldata->negate = TRUE;

   nodeseldata->nerrors = 0;
   nodeseldata->ncomps = 0;

   return SCIP_OKAY;
}

/** destructor of node selector to free user data (called when SCIP is exiting) */
static
SCIP_DECL_NODESELEXIT(nodeselExitDagger)
{
   int i=0;
   SCIP_NODESELDATA* nodeseldata;
   assert(scip != NULL);
   assert(nodesel != NULL);

   nodeseldata = SCIPnodeselGetData(nodesel);

   assert(nodeseldata->feasiblesols != NULL);
   for(i=0; i<nodeseldata->nfeasiblesols; i++){
      assert(nodeseldata->feasiblesols[i] != NULL);
      SCIP_CALL( SCIPfreeSolSelf(scip, &(nodeseldata->feasiblesols[i])) );
      SCIP_CALL( SCIPfeatFree(scip, &(nodeseldata->feasiblefeat[i])) );;
   }
   BMSfreeMemory(&(nodeseldata->feasiblesols));
   BMSfreeMemory(&(nodeseldata->feasiblefeat));
   BMSfreeMemory(&(nodeseldata->solflag));
   BMSfreeMemory(&(nodeseldata->feasiblescore));

   if( nodeseldata->trjfile1 != NULL)
   {
      fclose(nodeseldata->wfile);
      fclose(nodeseldata->trjfile1);
#ifndef LIBLINEAR
      fclose(nodeseldata->trjfile2);
#endif
   }

   if(nodeseldata->probfeats != NULL)
      BMSfreeMemory(&(nodeseldata->probfeats));

   assert(nodeseldata->feat != NULL);
   SCIP_CALL( SCIPfeatFree(scip, &nodeseldata->feat) );

#ifdef LIBLINEAR
   assert(nodeseldata->policy != NULL);
   SCIP_CALL( SCIPpolicyFree(scip, &nodeseldata->policy) );
#else
   assert(nodeseldata->policy != NULL);
   SCIP_CALL( SCIPkerasPolicyFree(scip, &nodeseldata->policy) );
#endif

   return SCIP_OKAY;
}

/** destructor of node selector to free user data (called when SCIP is exiting) */
static
SCIP_DECL_NODESELFREE(nodeselFreeDagger)
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
SCIP_DECL_NODESELSELECT(nodeselSelectDagger)
{
   SCIP_NODESELDATA* nodeseldata;
   SCIP_NODE** leaves;
   SCIP_NODE** children;
   SCIP_NODE** siblings;
   int nleaves;
   int nsiblings;
   int nchildren;
   SCIP_Bool optchild;
   SCIP_Real noise;
   int i;

   assert(nodesel != NULL);
   assert(strcmp(SCIPnodeselGetName(nodesel), NODESEL_NAME) == 0);
   assert(scip != NULL);
   assert(selnode != NULL);

   nodeseldata = SCIPnodeselGetData(nodesel);
   assert(nodeseldata != NULL);

   /* collect leaves, children and siblings data */
   SCIP_CALL( SCIPgetOpenNodesData(scip, &leaves, &children, &siblings, &nleaves, &nchildren, &nsiblings) );

   /* check newly created nodes */
   optchild = FALSE;
   for( i = 0; i < nchildren; i++)
   {
      // Check if memory is allocated for this node; if not, allocate
      SCIPnodeAllocSolFlag(children[i], nodeseldata->nfeasiblesols);

      // For now, doing this twice: Once here and once inside getSolIndex. Fix later
      SCIPnodeSetFeats(scip, children[i]);
      setNodeUbLb(children[i]);

      /* populate feats */
      SCIPcalcNodeselFeat(scip, children[i], nodeseldata->feat, \
                                    nodeseldata->probfeats, nodeseldata->probfeatsize);

      /* compute score */
#ifdef LIBLINEAR
      SCIPcalcNodeScore(children[i], nodeseldata->feat, nodeseldata->policy, nodeseldata->probfeatsize);
#else
      SCIPcalcKerasNodeScore(children[i], nodeseldata->feat, &nodeseldata->policy, nodeseldata->probfeatsize);
#endif

      if (nodeseldata->score_noise != 0){
         SCIP_Real cscore = SCIPnodeGetScore(children[i]);
         SCIP_Real score_with_noise;
         noise = getRandom(nodeseldata->score_noise);
         score_with_noise = cscore + noise;
         SCIPnodeSetScore(children[i], score_with_noise);
      }

      /* check optimality */
      if( ! SCIPnodeIsOptchecked(children[i]) )
      {
         SCIP_Longint solIdx;
         solIdx = getSolIndex(scip, children[i], nodeseldata->feasiblesols, 
                                    nodeseldata->solflag, nodeseldata->nfeasiblesols, nodeseldata->margin);

         /* Get feasible node feats and update index */
         if(solIdx != -1){
            nodeseldata->feasiblescore[solIdx] = SCIPnodeGetScore(children[i]);
            SCIPcalcNodeselFeat(scip, children[i], nodeseldata->feasiblefeat[solIdx], \
                                    nodeseldata->probfeats, nodeseldata->probfeatsize);
            SCIPnodeSetSolIdx(children[i], solIdx);
         }

         SCIPnodeSetOptchecked(children[i]);
      }

      /* Use the solution to set optimality */
      if (nodeseldata->beta != 0){
         float sample = getBinomial();
         if (sample < nodeseldata->beta){
           if(SCIPnodeIsOptimal(children[i])){
              SCIPnodeSetScore(children[i], 1);
           }
           else{
              SCIPnodeSetScore(children[i], 0);
           }
         }
      }

      if( SCIPnodeIsOptimal(children[i]) )
      {
#ifndef NDEBUG
         SCIPdebugMessage("opt node #%"SCIP_LONGINT_FORMAT"\n", SCIPnodeGetNumber(children[i]));
#endif
         nodeseldata->optnodenumber = SCIPnodeGetNumber(children[i]);
         optchild = TRUE;
      }
   }

   /* write examples */
   if( nodeseldata->trjfile1 != NULL )
   {
     for( i = 0; i < nchildren; i++){
            createFeatDiff(scip, children[i], nodeseldata);
     }
     if(optchild)
     {
         /* Complete pair-wise feature created only if there is an 
            optimal solution among new children */
         for( i = 0; i < nsiblings; i++ ){
            createFeatDiff(scip, siblings[i], nodeseldata);
         }
         for( i = 0; i < nleaves; i++ ){
            createFeatDiff(scip, leaves[i], nodeseldata);
         }
      }
   }

   *selnode = SCIPgetBestNode(scip);

   return SCIP_OKAY;
}


/** node comparison method of dagger node selector */
static
SCIP_DECL_NODESELCOMP(nodeselCompDagger)
{  /*lint --e{715}*/
   SCIP_Real score1;
   SCIP_Real score2;
   SCIP_Bool isopt1;
   SCIP_Bool isopt2;
   SCIP_NODESELDATA* nodeseldata;
   int result;

   assert(nodesel != NULL);
   assert(strcmp(SCIPnodeselGetName(nodesel), NODESEL_NAME) == 0);
   assert(scip != NULL);

   assert(SCIPnodeIsOptchecked(node1) == TRUE);
   assert(SCIPnodeIsOptchecked(node2) == TRUE);

   score1 = SCIPnodeGetScore(node1);
   score2 = SCIPnodeGetScore(node2);

   assert(score1 != 0);
   assert(score2 != 0);

   if( SCIPisGT(scip, score1, score2) )
      result = -1;
   else if( SCIPisLT(scip, score1, score2) )
      result = +1;
   else
   {
      int depth1;
      int depth2;

      depth1 = SCIPnodeGetDepth(node1);
      depth2 = SCIPnodeGetDepth(node2);
      if( depth1 > depth2 )
         result = -1;
      else if( depth1 < depth2 )
         result = +1;
      else
      {
         SCIP_Real lowerbound1;
         SCIP_Real lowerbound2;

         lowerbound1 = SCIPnodeGetLowerbound(node1);
         lowerbound2 = SCIPnodeGetLowerbound(node2);
         if( SCIPisLT(scip, lowerbound1, lowerbound2) )
            result = -1;
         else if( SCIPisGT(scip, lowerbound1, lowerbound2) )
            result = +1;
         else
            result = 0;
      }
   }

   nodeseldata = SCIPnodeselGetData(nodesel);
   assert(nodeseldata != NULL);
   isopt1 = SCIPnodeIsOptimal(node1);
   isopt2 = SCIPnodeIsOptimal(node2);
   if ((isopt1 && !isopt2) && (result == 1))
      nodeseldata->nerrors++;
   else if ((!isopt1 && isopt2) && (result == -1))
      nodeseldata->nerrors++;
   else if (isopt1 && isopt2){
      /* both optimal */
      int csolidx1 = SCIPnodeGetSolIdx(node1);
      int csolidx2 = SCIPnodeGetSolIdx(node2);
      SCIP_Real cobj1 = SCIPsolGetOrigObj(nodeseldata->feasiblesols[csolidx1]);
      SCIP_Real cobj2 = SCIPsolGetOrigObj(nodeseldata->feasiblesols[csolidx2]);
      // Node one objective is better
      if ((cobj1 < cobj2) && (result == 1))
            nodeseldata->nerrors++;
      else if ((cobj1 > cobj2) && (result == -1))
            nodeseldata->nerrors++;
   }

   /* don't count the case when neither node is opt, in which case it will always be correct */
   if( isopt1 || isopt2 )
      nodeseldata->ncomps++;

   return result;
}

static
SCIP_RETCODE createFeatDiff(
   SCIP*                scip,
   SCIP_NODE*           node,
   SCIP_NODESELDATA*    nodeseldata
)
{
   int csolidx = SCIPnodeGetSolIdx(node);
   int cnum    = SCIPnodeGetNumber(node);
   int j = 0, scale;
   SCIP_Longint cscore = SCIPnodeGetScore(node);

   SCIPcalcNodeselFeat(scip, node, nodeseldata->feat, \
                          nodeseldata->probfeats, nodeseldata->probfeatsize);

   for(j=0; j<nodeseldata->nfeasiblesols; j++)
   {
      int feasnum = SCIPfeatGetNumber(nodeseldata->feasiblefeat[j]);
      if (!(nodeseldata->solflag[j])){
         continue;
      }
      else if ((feasnum == -1) || (feasnum == cnum)){
         // No node belonging to this solution or child belongs to the same sol
         continue;
      }
      else{
         nodeseldata->negate ^= 1;
         SCIP_Longint cfeasScore = nodeseldata->feasiblescore[j];

         /* Feasible solution should be something other than the initialization */
         assert(nodeseldata->feasiblescore[j] != SCIP_LONGINT_MIN);

         if (csolidx == -1){
            /* condition when the node does not belong to an optimal path */
            //scale = 1+(cfeasScore < cscore);

            SCIPfeatDiffLIBSVMPrint(scip, nodeseldata->trjfile1, nodeseldata->trjfile2,\
                                          nodeseldata->wfile, nodeseldata->feasiblefeat[j], \
                                          nodeseldata->feat, 1, nodeseldata->scale, nodeseldata->negate);
         }
         else{
            /* optimal path */
            SCIP_Real cobj = SCIPsolGetOrigObj(nodeseldata->feasiblesols[csolidx]);
            SCIP_Real otherobj = SCIPsolGetOrigObj(nodeseldata->feasiblesols[j]);
            SCIP_Bool label = (cobj > otherobj)? 1 : -1;
            assert(cscore != nodeseldata->feasiblescore[csolidx]);

            /*
            if (label == 1)
                  // feasible solution with index j ranked higher than node 
                  scale = 1+(cfeasScore < cscore);
            else
                  // node ranked higher than the feasible solution 
                  scale = 1+(cfeasScore > cscore);
            */

            SCIPfeatDiffLIBSVMPrint(scip, nodeseldata->trjfile1, nodeseldata->trjfile2, \
                                          nodeseldata->wfile, nodeseldata->feasiblefeat[j], \
                                          nodeseldata->feat, label, nodeseldata->scale, nodeseldata->negate);
         }
      }
   }
   return SCIP_OKAY;
}

/*
 * node selector specific interface methods
 */

/** creates the uct node selector and includes it in SCIP */
SCIP_RETCODE SCIPincludeNodeselDagger(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_NODESELDATA* nodeseldata;
   SCIP_NODESEL* nodesel;

   /* create dagger node selector data */
   SCIP_CALL( SCIPallocBlockMemory(scip, &nodeseldata) );

   nodesel = NULL;
   nodeseldata->feasiblesols = NULL;
   nodeseldata->solfname = NULL;
   nodeseldata->normfname = NULL;
   nodeseldata->trjfname = NULL;
   nodeseldata->polfname = NULL;
   nodeseldata->probfeatsfname = NULL;

   /* use SCIPincludeNodeselBasic() plus setter functions if you want to set callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeNodeselBasic(scip, &nodesel, NODESEL_NAME, NODESEL_DESC, NODESEL_STDPRIORITY,
          NODESEL_MEMSAVEPRIORITY, nodeselSelectDagger, nodeselCompDagger, nodeseldata) );

   assert(nodesel != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetNodeselCopy(scip, nodesel, NULL) );
   SCIP_CALL( SCIPsetNodeselInit(scip, nodesel, nodeselInitDagger) );
   SCIP_CALL( SCIPsetNodeselExit(scip, nodesel, nodeselExitDagger) );
   SCIP_CALL( SCIPsetNodeselFree(scip, nodesel, nodeselFreeDagger) );

   /* add dagger node selector parameters */
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodeselection/"NODESEL_NAME"/solfname",
         "name of the optimal solution file",
         &nodeseldata->solfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodeselection/"NODESEL_NAME"/normfname",
         "name of the normalization file",
         &nodeseldata->normfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodeselection/"NODESEL_NAME"/trjfname",
         "name of the file to write node selection trajectories",
         &nodeseldata->trjfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodeselection/"NODESEL_NAME"/polfname",
         "name of the policy model file",
         &nodeseldata->polfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddStringParam(scip,
         "nodeselection/"NODESEL_NAME"/probfeatsfname",
         "name of the file with prob specific features",
         &nodeseldata->probfeatsfname, FALSE, DEFAULT_FILENAME, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip, 
         "nodeselection/"NODESEL_NAME"/ogapThreshold",
         "optimality gap threshold of solutions to consider", 
         &nodeseldata->ogapThreshold, FALSE, 0, 0, 1, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip, 
         "nodeselection/"NODESEL_NAME"/score_noise",
         "Noise added to the score of each node", 
         &nodeseldata->score_noise, FALSE, 0, 0, 1, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip, 
         "nodeselection/"NODESEL_NAME"/scale",
         "Scale the weights of the data points", 
         &nodeseldata->scale, FALSE, 1, 0.001, SCIP_REAL_MAX, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip, 
         "nodeselection/"NODESEL_NAME"/margin",
         "Scale the weights of the data points", 
         &nodeseldata->margin, FALSE, 0, 0, 0.05, NULL, NULL) );
   SCIP_CALL( SCIPaddRealParam(scip, 
         "nodeselection/"NODESEL_NAME"/beta",
         "Beta value", 
         &nodeseldata->beta, FALSE, 0, 0, 1, NULL, NULL) );

   return SCIP_OKAY;
}
