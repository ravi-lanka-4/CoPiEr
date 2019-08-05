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

/**@file   pub_nodepru.h
 * @ingroup PUBLICMETHODS
 * @brief  public methods for node pruners
 * @author He He 
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_PUB_NODEPRU_H__
#define __SCIP_PUB_NODEPRU_H__


#include "scip/def.h"
#include "scip/type_nodepru.h"

#ifdef __cplusplus
extern "C" {
#endif

/** gets name of node pruner */
EXTERN
const char* SCIPnodepruGetName(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   );

/** gets description of node pruner */
EXTERN
const char* SCIPnodepruGetDesc(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   );

/** gets priority of node pruner in standard mode */
EXTERN
int SCIPnodepruGetStdPriority(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   );

/** gets priority of node pruner in memory saving mode */
EXTERN
int SCIPnodepruGetMemsavePriority(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   );

/** gets user data of node pruner */
EXTERN
SCIP_NODEPRUDATA* SCIPnodepruGetData(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   );

/** sets user data of node pruner; user has to free old data in advance! */
EXTERN
void SCIPnodepruSetData(
   SCIP_NODEPRU*         nodepru,            /**< node pruner */
   SCIP_NODEPRUDATA*     nodeprudata         /**< new node pruner user data */
   );

/** is node pruner initialized? */
EXTERN
SCIP_Bool SCIPnodepruIsInitialized(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   );

/** gets time in seconds used in this node pruner for setting up for next stages */
EXTERN
SCIP_Real SCIPnodepruGetSetupTime(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   );

/** gets time in seconds used in this node pruner */
EXTERN
SCIP_Real SCIPnodepruGetTime(
   SCIP_NODEPRU*         nodepru             /**< node pruner */
   );

#ifdef __cplusplus
}
#endif

#endif
