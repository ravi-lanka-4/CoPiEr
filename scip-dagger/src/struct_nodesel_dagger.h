#ifndef STRUCT_NODESEL_DAGGER_H
#define STRUCT_NODESEL_DAGGER_H
#include "scip/scip.h"
#include "feat.h"

/** default values for user parameters */
#define DEFAULT_WEIGHT          0.01    /**< weight of node visits in UCT score */
#define DEFAULT_NODELIMIT       2500    /**< limit of node selections after which UCT node selection is turned off */
#define DEFAULT_USEESTIMATE     FALSE   /**< should the estimate (TRUE) or the lower bound of a node be used for UCT score? */
#define INITIALSIZE             2048    /**< initial size of node visits array (increased dynamically if required) */
#define MAXNODELIMIT            1000000 /**< the maximum value for user parameter nodelimit */

/** node selector data */
struct SCIP_NodeselData
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
   FILE*              trjfile1;           /** select is a ranking function so write them separately */
   FILE*              trjfile2;
   SCIP_FEAT*         feat;
   SCIP_Longint       optnodenumber;      /**< successively assigned number of the node */
   SCIP_Bool          negate;
   int                nerrors;            /**< number of wrong ranking of a pair of nodes */
   int                ncomps;             /**< total number of comparisons */
   SCIP_Real          scale;

   /* Optimal traces */
   SCIP_Real          ogapThreshold;      /**< Treshold to consider the solutions */
   int                isTrace;
   SCIP_Longint       nfeasiblesols;
   SCIP_SOL**         feasiblesols;       /**< optimal solution */
   SCIP_FEAT**        feasiblefeat;
   SCIP_Bool*         solflag;
   SCIP_Longint*      feasiblescore;
   SCIP_Real          score_noise;
   SCIP_Real          margin;

   int*               nodevisits;         /**< array to store the number of node visits so far for every node */
   SCIP_Real          weight;             /**< weight of node visits in UCT score */
   int                nodelimit;          /**< limit of node selections after which UCT node selection is turned off */
   int                sizenodevisits;     /**< the size of the visits array */
   int                nselections;        /**< counter for the number of node selections */
   int                origstdpriority;    /**< priority of node selector when starting branch and bound */
   SCIP_Bool          useestimate;        /**< should the estimate (TRUE) or the lower bound of a node be used for UCT score? */

   /* Dagger percentage */
   SCIP_Real          beta;               /* Probability of using the expert opinion */

   /* Problem related features */
   char*              probfeatsfname;
   SCIP_Real*         probfeats;
   int                probfeatsize;

};

#endif