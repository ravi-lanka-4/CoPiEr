/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2014 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   nodepru.c
 * @brief  methods for node pruners
 * @author He He 
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>
#include <string.h>

#include "scip/def.h"
#include "scip/set.h"
#include "scip/clock.h"
#include "scip/stat.h"
#include "scip/paramset.h"
#include "scip/scip.h"
#include "scip/nodepru.h"
#include "scip/pub_message.h"
#include "scip/pub_misc.h"

#include "scip/struct_nodepru.h"


/*
 * node pruner methods 
 */

/** method to call, when the standard mode priority of a node pruner was changed */
static
SCIP_DECL_PARAMCHGD(paramChgdNodepruStdPriority)
{  /*lint --e{715}*/
   SCIP_PARAMDATA* paramdata;

   paramdata = SCIPparamGetData(param);
   assert(paramdata != NULL);

   /* use SCIPsetNodepruStdPriority() to mark the nodeprus unsorted */
   SCIP_CALL( SCIPsetNodepruStdPriority(scip, (SCIP_NODEPRU*)paramdata, SCIPparamGetInt(param)) ); /*lint !e740*/

   return SCIP_OKAY;
}

/** method to call, when the memory saving mode priority of a node pruner was changed */
static
SCIP_DECL_PARAMCHGD(paramChgdNodepruMemsavePriority)
{  /*lint --e{715}*/
   SCIP_PARAMDATA* paramdata;

   paramdata = SCIPparamGetData(param);
   assert(paramdata != NULL);

   /* use SCIPsetNodepruMemsavePriority() to mark the nodeprus unsorted */
   SCIP_CALL( SCIPsetNodepruMemsavePriority(scip, (SCIP_NODEPRU*)paramdata, SCIPparamGetInt(param)) ); /*lint !e740*/

   return SCIP_OKAY;
}

/** copies the given node pruner to a new scip */
SCIP_RETCODE SCIPnodepruCopyInclude(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set                 /**< SCIP_SET of SCIP to copy to */
   )
{
   assert(nodepru != NULL);
   assert(set != NULL);
   assert(set->scip != NULL);

   if( nodepru->nodeprucopy != NULL )
   {
      SCIPdebugMessage("including node pruner %s in subscip %p\n", SCIPnodepruGetName(nodepru), (void*)set->scip);
      SCIP_CALL( nodepru->nodeprucopy(set->scip, nodepru) );
   }
   return SCIP_OKAY;
}

/** creates a node pruner */
SCIP_RETCODE SCIPnodepruCreate(
   SCIP_NODEPRU**        nodepru,            /**< pointer to store node pruner */
   SCIP_SET*             set,                /**< global SCIP settings */
   SCIP_MESSAGEHDLR*     messagehdlr,        /**< message handler */
   BMS_BLKMEM*           blkmem,             /**< block memory for parameter settings */
   const char*           name,               /**< name of node pruner */
   const char*           desc,               /**< description of node pruner */
   int                   stdpriority,        /**< priority of the node pruner in standard mode */
   int                   memsavepriority,    /**< priority of the node pruner in memory saving mode */
   SCIP_DECL_NODEPRUCOPY ((*nodeprucopy)),   /**< copy method of node pruner or NULL if you don't want to copy your plugin into sub-SCIPs */
   SCIP_DECL_NODEPRUFREE ((*nodeprufree)),   /**< destructor of node pruner */
   SCIP_DECL_NODEPRUINIT ((*nodepruinit)),   /**< initialize node pruner */
   SCIP_DECL_NODEPRUEXIT ((*nodepruexit)),   /**< deinitialize node pruner */
   SCIP_DECL_NODEPRUINITSOL((*nodepruinitsol)),/**< solving process initialization method of node pruner */
   SCIP_DECL_NODEPRUEXITSOL((*nodepruexitsol)),/**< solving process deinitialization method of node pruner */
   SCIP_DECL_NODEPRUPRUNE((*nodepruprune)),/**< node pruning method */
   SCIP_NODEPRUDATA*     nodeprudata         /**< node pruner data */
   )
{
   char paramname[SCIP_MAXSTRLEN];
   char paramdesc[SCIP_MAXSTRLEN];

   assert(nodepru != NULL);
   assert(name != NULL);
   assert(desc != NULL);
   assert(nodepruprune != NULL);

   SCIP_ALLOC( BMSallocMemory(nodepru) );
   SCIP_ALLOC( BMSduplicateMemoryArray(&(*nodepru)->name, name, strlen(name)+1) );
   SCIP_ALLOC( BMSduplicateMemoryArray(&(*nodepru)->desc, desc, strlen(desc)+1) );
   (*nodepru)->nodeprucopy = nodeprucopy;
   (*nodepru)->nodeprufree = nodeprufree;
   (*nodepru)->nodepruinit = nodepruinit;
   (*nodepru)->nodepruexit = nodepruexit;
   (*nodepru)->nodepruinitsol = nodepruinitsol;
   (*nodepru)->nodepruexitsol = nodepruexitsol;
   (*nodepru)->nodepruprune = nodepruprune;
   (*nodepru)->nodeprudata = nodeprudata;
   (*nodepru)->initialized = FALSE;
   /* create clocks */
   SCIP_CALL( SCIPclockCreate(&(*nodepru)->setuptime, SCIP_CLOCKTYPE_DEFAULT) );
   SCIP_CALL( SCIPclockCreate(&(*nodepru)->nodeprutime, SCIP_CLOCKTYPE_DEFAULT) );

   /* add parameters */
   (void) SCIPsnprintf(paramname, SCIP_MAXSTRLEN, "nodepruning/%s/stdpriority", name);
   (void) SCIPsnprintf(paramdesc, SCIP_MAXSTRLEN, "priority of node pruning rule <%s> in standard mode", name);
   SCIP_CALL( SCIPsetAddIntParam(set, messagehdlr, blkmem, paramname, paramdesc,
                  &(*nodepru)->stdpriority, FALSE, stdpriority, INT_MIN/4, INT_MAX/4,
                  paramChgdNodepruStdPriority, (SCIP_PARAMDATA*)(*nodepru)) ); /*lint !e740*/

   (void) SCIPsnprintf(paramname, SCIP_MAXSTRLEN, "nodepruning/%s/memsavepriority", name);
   (void) SCIPsnprintf(paramdesc, SCIP_MAXSTRLEN, "priority of node pruning rule <%s> in memory saving mode", name);
   SCIP_CALL( SCIPsetAddIntParam(set, messagehdlr, blkmem, paramname, paramdesc,
                  &(*nodepru)->memsavepriority, TRUE, memsavepriority, INT_MIN/4, INT_MAX/4,
                  paramChgdNodepruMemsavePriority, (SCIP_PARAMDATA*)(*nodepru)) ); /*lint !e740*/

   return SCIP_OKAY;
}

/** frees memory of node pruner */
SCIP_RETCODE SCIPnodepruFree(
   SCIP_NODEPRU**        nodepru,            /**< pointer to node pruner data structure */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(nodepru != NULL);
   assert(*nodepru != NULL);
   assert(!(*nodepru)->initialized);
   assert(set != NULL);

   /* call destructor of node pruner */
   if( (*nodepru)->nodeprufree != NULL )
   {
      SCIP_CALL( (*nodepru)->nodeprufree(set->scip, *nodepru) );
   }

   /* free clocks */
   SCIPclockFree(&(*nodepru)->nodeprutime);
   SCIPclockFree(&(*nodepru)->setuptime);

   BMSfreeMemoryArray(&(*nodepru)->name);
   BMSfreeMemoryArray(&(*nodepru)->desc);
   BMSfreeMemory(nodepru);

   return SCIP_OKAY;
}

/** initializes node pruner */
SCIP_RETCODE SCIPnodepruInit(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(nodepru != NULL);
   assert(set != NULL);

   if( nodepru->initialized )
   {
      SCIPerrorMessage("node pruner <%s> already initialized", nodepru->name);
      return SCIP_INVALIDCALL;
   }

   if( set->misc_resetstat )
   {
      SCIPclockReset(nodepru->setuptime);
      SCIPclockReset(nodepru->nodeprutime);
   }

   if( nodepru->nodepruinit != NULL )
   {
      /* start timing */
      SCIPclockStart(nodepru->setuptime, set);

      SCIP_CALL( nodepru->nodepruinit(set->scip, nodepru) );

      /* stop timing */
      SCIPclockStop(nodepru->setuptime, set);
   }
   nodepru->initialized = TRUE;

   return SCIP_OKAY;
}

/** deinitializes node pruner */
SCIP_RETCODE SCIPnodepruExit(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(nodepru != NULL);
   assert(set != NULL);

   if( !nodepru->initialized )
   {
      SCIPerrorMessage("node pruner <%s> not initialized\n", nodepru->name);
      return SCIP_INVALIDCALL;
   }

   if( nodepru->nodepruexit != NULL )
   {
      /* start timing */
      SCIPclockStart(nodepru->setuptime, set);

      SCIP_CALL( nodepru->nodepruexit(set->scip, nodepru) );

      /* stop timing */
      SCIPclockStop(nodepru->setuptime, set);
   }
   nodepru->initialized = FALSE;

   return SCIP_OKAY;
}

/** informs node pruner that the branch and bound process is being started */
SCIP_RETCODE SCIPnodepruInitsol(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(nodepru != NULL);
   assert(set != NULL);

   /* call solving process initialization method of node pruner */
   if( nodepru->nodepruinitsol != NULL )
   {
      /* start timing */
      SCIPclockStart(nodepru->setuptime, set);

      SCIP_CALL( nodepru->nodepruinitsol(set->scip, nodepru) );

      /* stop timing */
      SCIPclockStop(nodepru->setuptime, set);
   }

   return SCIP_OKAY;
}

/** informs node pruner that the branch and bound process data is being freed */
SCIP_RETCODE SCIPnodepruExitsol(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(nodepru != NULL);
   assert(set != NULL);

   /* call solving process deinitialization method of node pruner */
   if( nodepru->nodepruexitsol != NULL )
   {
      /* start timing */
      SCIPclockStart(nodepru->setuptime, set);

      SCIP_CALL( nodepru->nodepruexitsol(set->scip, nodepru) );

      /* stop timing */
      SCIPclockStop(nodepru->setuptime, set);
   }

   return SCIP_OKAY;
}

/** select next node to be processed */
SCIP_RETCODE SCIPnodepruPrune(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set,                /**< global SCIP settings */
   SCIP_NODE*            node,               /**< pointer to store node to be processed next */
   SCIP_Bool*            prune               /**< pointer to store node to be processed next */
   )
{
   assert(nodepru != NULL);
   assert(nodepru->nodepruprune != NULL);
   assert(set != NULL);
   assert(node != NULL);

   /* start timing */
   SCIPclockStart(nodepru->nodeprutime, set);

   SCIP_CALL( nodepru->nodepruprune(set->scip, nodepru, node, prune) );

   /* stop timing */
   SCIPclockStop(nodepru->nodeprutime, set);

   return SCIP_OKAY;
}

/** gets name of node pruner */
const char* SCIPnodepruGetName(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   )
{
   assert(nodepru != NULL);

   return nodepru->name;
}

/** gets description of node pruner */
const char* SCIPnodepruGetDesc(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   )
{
   assert(nodepru != NULL);

   return nodepru->desc;
}

/** gets priority of node pruner in standard mode */
int SCIPnodepruGetStdPriority(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   )
{
   assert(nodepru != NULL);

   return nodepru->stdpriority;
}

/** gets priority of node pruner in standard mode */
void SCIPnodepruSetStdPriority(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set,                /**< global SCIP settings */
   int                   priority            /**< new priority of the node pruner */
   )
{
   assert(nodepru != NULL);
   assert(set != NULL);

   nodepru->stdpriority = priority;
   set->nodepru = NULL;
}

/** gets priority of node pruner in memory saving mode */
int SCIPnodepruGetMemsavePriority(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   )
{
   assert(nodepru != NULL);

   return nodepru->memsavepriority;
}

/** sets priority of node pruner in memory saving mode */
void SCIPnodepruSetMemsavePriority(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set,                /**< global SCIP settings */
   int                   priority            /**< new priority of the node pruner */
   )
{
   assert(nodepru != NULL);
   assert(set != NULL);
   
   nodepru->memsavepriority = priority;
   set->nodepru = NULL;
}

/** gets user data of node pruner */
SCIP_NODEPRUDATA* SCIPnodepruGetData(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   )
{
   assert(nodepru != NULL);

   return nodepru->nodeprudata;
}

/** sets user data of node pruner; user has to free old data in advance! */
void SCIPnodepruSetData(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_NODEPRUDATA*     nodeprudata         /**< new node pruner user data */
   )
{
   assert(nodepru != NULL);

   nodepru->nodeprudata = nodeprudata;
}

/* new callback/method setter methods */

/** sets copy method of node pruner */
void SCIPnodepruSetCopy(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUCOPY ((*nodeprucopy))    /**< copy method of node pruner or NULL if you don't want to copy your plugin into sub-SCIPs */
   )
{
   assert(nodepru != NULL);

   nodepru->nodeprucopy = nodeprucopy;
}

/** sets destructor method of node pruner */
void SCIPnodepruSetFree(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUFREE ((*nodeprufree))    /**< destructor of node pruner */
   )
{
   assert(nodepru != NULL);

   nodepru->nodeprufree = nodeprufree;
}

/** sets initialization method of node pruner */
void SCIPnodepruSetInit(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUINIT ((*nodepruinit))    /**< initialize node pruner */
   )
{
   assert(nodepru != NULL);

   nodepru->nodepruinit = nodepruinit;
}

/** sets deinitialization method of node pruner */
void SCIPnodepruSetExit(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUEXIT ((*nodepruexit))    /**< deinitialize node pruner */
   )
{
   assert(nodepru != NULL);

   nodepru->nodepruexit = nodepruexit;
}

/** sets solving process initialization method of node pruner */
void SCIPnodepruSetInitsol(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUINITSOL ((*nodepruinitsol))/**< solving process initialization method of node pruner */
   )
{
   assert(nodepru != NULL);

   nodepru->nodepruinitsol = nodepruinitsol;
}

/** sets solving process deinitialization method of node pruner */
void SCIPnodepruSetExitsol(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUEXITSOL ((*nodepruexitsol))/**< solving process deinitialization method of node pruner */
   )
{
   assert(nodepru != NULL);

   nodepru->nodepruexitsol = nodepruexitsol;
}

/** is node pruner initialized? */
SCIP_Bool SCIPnodepruIsInitialized(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   )
{
   assert(nodepru != NULL);

   return nodepru->initialized;
}

/** gets time in seconds used in this node pruner for setting up for next stages */
SCIP_Real SCIPnodepruGetSetupTime(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   )
{
   assert(nodepru != NULL);

   return SCIPclockGetTime(nodepru->setuptime);
}

/** gets time in seconds used in this node pruner */
SCIP_Real SCIPnodepruGetTime(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   )
{
   assert(nodepru != NULL);

   return SCIPclockGetTime(nodepru->nodeprutime);
}
