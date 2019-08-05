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

/**@file   type_nodepru.h
 * @ingroup TYPEDEFINITIONS
 * @brief  type definitions for node pruners 
 * @author He He 
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_TYPE_NODEPRU_H__
#define __SCIP_TYPE_NODEPRU_H__

#include "scip/def.h"
#include "scip/type_retcode.h"
#include "scip/type_scip.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SCIP_Nodepru SCIP_NODEPRU;         /**< node pruner data structure */
typedef struct SCIP_NodepruData SCIP_NODEPRUDATA; /**< node pruner specific data */


/** copy method for node pruner plugins (called when SCIP copies plugins)
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - nodepru         : the node pruner itself
 */
#define SCIP_DECL_NODEPRUCOPY(x) SCIP_RETCODE x (SCIP* scip, SCIP_NODEPRU* nodepru)


/** destructor of node pruner to free user data (called when SCIP is exiting)
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - nodepru         : the node pruner itself
 */
#define SCIP_DECL_NODEPRUFREE(x) SCIP_RETCODE x (SCIP* scip, SCIP_NODEPRU* nodepru)

/** initialization method of node pruner (called after problem was transformed)
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - nodepru         : the node pruner itself
 */
#define SCIP_DECL_NODEPRUINIT(x) SCIP_RETCODE x (SCIP* scip, SCIP_NODEPRU* nodepru)

/** deinitialization method of node pruner (called before transformed problem is freed)
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - nodepru         : the node pruner itself
 */
#define SCIP_DECL_NODEPRUEXIT(x) SCIP_RETCODE x (SCIP* scip, SCIP_NODEPRU* nodepru)

/** solving process initialization method of node pruner (called when branch and bound process is about to begin)
 *
 *  This method is called when the presolving was finished and the branch and bound process is about to begin.
 *  The node pruner may use this call to initialize its branch and bound specific data.
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - nodepru         : the node pruner itself
 */
#define SCIP_DECL_NODEPRUINITSOL(x) SCIP_RETCODE x (SCIP* scip, SCIP_NODEPRU* nodepru)

/** solving process deinitialization method of node pruner (called before branch and bound process data is freed)
 *
 *  This method is called before the branch and bound process is freed.
 *  The node pruner should use this call to clean up its branch and bound data.
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - nodepru         : the node pruner itself
 */
#define SCIP_DECL_NODEPRUEXITSOL(x) SCIP_RETCODE x (SCIP* scip, SCIP_NODEPRU* nodepru)

/** node pruning method of node pruner
 *
 *  This method is called to decide whether to prune a node selected by the node selector and is not cutoff.
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - nodepru         : the node pruner itself
 *  - node            : the node under decision 
 *  - prune           : pointer to store the pruning decision 
 *
 *  possible return values for *prune:
 *  - FALSE: don't prune the node 
 *  - TRUE : prune the node 
 */
#define SCIP_DECL_NODEPRUPRUNE(x) SCIP_RETCODE x (SCIP* scip, SCIP_NODEPRU* nodepru, SCIP_NODE* node, SCIP_Bool* prune)

#ifdef __cplusplus
}
#endif

#endif
