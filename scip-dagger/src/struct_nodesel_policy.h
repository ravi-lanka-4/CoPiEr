#ifndef STRUCT_NODESEL_POLICY_H
#define STRUCT_NODESEL_POLICY_H

#include "scip/scip.h"
#include "feat.h"

/** default values for user parameters */
#define DEFAULT_WEIGHT          0.05    /**< weight of node visits in UCT score */
#define DEFAULT_NODELIMIT       2500    /**< limit of node selections after which UCT node selection is turned off */
#define DEFAULT_USEESTIMATE     FALSE   /**< should the estimate (TRUE) or the lower bound of a node be used for UCT score? */
#define INITIALSIZE             2048    /**< initial size of node visits array (increased dynamically if required) */
#define MAXNODELIMIT            1000000 /**< the maximum value for user parameter nodelimit */

/** node selector data */
struct SCIP_NodeselData
{
   char*              polfname;           /**< name of the solution file */
   char*              normfname;
#ifdef LIBLINEAR
   SCIP_POLICY*       policy;
#else
   void *             policy;
#endif
   SCIP_FEAT*         feat;
   SCIP_Real          score_noise;
   char*              probfeatsfname;

   /* Problem specific features */
   SCIP_Real*         probfeats;
   int                probfeatsize;

   int*               nodevisits;         /**< array to store the number of node visits so far for every node */
   SCIP_Real          weight;             /**< weight of node visits in UCT score */
   int                nodelimit;          /**< limit of node selections after which UCT node selection is turned off */
   int                sizenodevisits;     /**< the size of the visits array */
   int                nselections;        /**< counter for the number of node selections */
   int                origstdpriority;    /**< priority of node selector when starting branch and bound */
   SCIP_Bool          useestimate;        /**< should the estimate (TRUE) or the lower bound of a node be used for UCT score? */
};

#endif