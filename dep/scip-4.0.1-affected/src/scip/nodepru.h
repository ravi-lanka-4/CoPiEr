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

/**@file   nodepru.h
 * @brief  internal methods for node pruners 
 * @author He He 
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_NODEPRU_H__
#define __SCIP_NODEPRU_H__


#include "scip/def.h"
#include "blockmemshell/memory.h"
#include "scip/type_retcode.h"
#include "scip/type_set.h"
#include "scip/type_stat.h"
#include "scip/pub_nodepru.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * node pruner methods
 */

/** copies the given node pruner to a new scip */
extern
SCIP_RETCODE SCIPnodepruCopyInclude(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set                 /**< SCIP_SET of SCIP to copy to */
   );

/** creates a node pruner */
extern
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
   SCIP_DECL_NODEPRUPRUNE((*nodepruprune)),/**< node selection method */
   SCIP_NODEPRUDATA*     nodeprudata         /**< node pruner data */
   );

/** frees memory of node pruner */
extern
SCIP_RETCODE SCIPnodepruFree(
   SCIP_NODEPRU**        nodepru,            /**< pointer to node pruner data structure */
   SCIP_SET*             set                 /**< global SCIP settings */
   );

/** initializes node pruner */
extern
SCIP_RETCODE SCIPnodepruInit(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set                 /**< global SCIP settings */
   );

/** deinitializes node pruner */
extern
SCIP_RETCODE SCIPnodepruExit(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set                 /**< global SCIP settings */
   );

/** informs node pruner that the branch and bound process is being started */
extern
SCIP_RETCODE SCIPnodepruInitsol(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set                 /**< global SCIP settings */
   );

/** informs node pruner that the branch and bound process data is being freed */
extern
SCIP_RETCODE SCIPnodepruExitsol(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set                 /**< global SCIP settings */
   );

/** decide whether to prune the node*/
extern
SCIP_RETCODE SCIPnodepruPrune(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set,                /**< global SCIP settings */
   SCIP_NODE*            node,               /**< pointer to the node under decision */
   SCIP_Bool*            prune               /**< pointer to store pruning decision*/
   );

/** sets priority of node pruner in standard mode */
extern
void SCIPnodepruSetStdPriority(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set,                /**< global SCIP settings */
   int                   priority            /**< new priority of the node pruner */
   );

/** sets priority of node pruner in memory saving mode */
extern
void SCIPnodepruSetMemsavePriority(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_SET*             set,                /**< global SCIP settings */
   int                   priority            /**< new priority of the node pruner */
   );

/** sets copy method of node pruner */
extern
void SCIPnodepruSetCopy(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUCOPY ((*nodeprucopy))    /**< copy method of node pruner or NULL if you don't want to copy your plugin into sub-SCIPs */
   );

/** sets destructor method of node pruner */
extern
void SCIPnodepruSetFree(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUFREE ((*nodeprufree))    /**< destructor of node pruner */
   );

/** sets initialization method of node pruner */
extern
void SCIPnodepruSetInit(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUINIT ((*nodepruinit))    /**< initialize node pruner */
   );

/** sets deinitialization method of node pruner */
extern
void SCIPnodepruSetExit(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUEXIT ((*nodepruexit))    /**< deinitialize node pruner */
   );

/** sets solving process initialization method of node pruner */
extern
void SCIPnodepruSetInitsol(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUINITSOL ((*nodepruinitsol))/**< solving process initialization method of node pruner */
   );

/** sets solving process deinitialization method of node pruner */
extern
void SCIPnodepruSetExitsol(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_DECL_NODEPRUEXITSOL ((*nodepruexitsol))/**< solving process deinitialization method of node pruner */
   );


#ifdef __cplusplus
}
#endif

#endif
