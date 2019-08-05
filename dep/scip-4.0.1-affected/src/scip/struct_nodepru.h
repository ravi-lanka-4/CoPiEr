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

/**@file   struct_nodepru.h
 * @brief  datastructures for node pruners
 * @author He He 
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_STRUCT_NODEPRU_H__
#define __SCIP_STRUCT_NODEPRU_H__


#include "scip/def.h"
#include "scip/type_nodepru.h"

#ifdef __cplusplus
extern "C" {
#endif

/** node pruner */
struct SCIP_Nodepru
{
   char*                 name;               /**< name of node pruner */
   char*                 desc;               /**< description of node pruner */
   SCIP_DECL_NODEPRUCOPY ((*nodeprucopy));   /**< copy method of node pruner or NULL if you don't want to copy your plugin into sub-SCIPs */
   SCIP_DECL_NODEPRUFREE ((*nodeprufree));   /**< destructor of node pruner */
   SCIP_DECL_NODEPRUINIT ((*nodepruinit));   /**< initialize node pruner */
   SCIP_DECL_NODEPRUEXIT ((*nodepruexit));   /**< deinitialize node pruner */
   SCIP_DECL_NODEPRUINITSOL((*nodepruinitsol));/**< solving process initialization method of node pruner */
   SCIP_DECL_NODEPRUEXITSOL((*nodepruexitsol));/**< solving process deinitialization method of node pruner */
   SCIP_DECL_NODEPRUPRUNE((*nodepruprune));/**< node pruning method */
   SCIP_CLOCK*           setuptime;          /**< time spend for setting up this node pruner for the next stages */
   SCIP_CLOCK*           nodeprutime;        /**< node pruner execution time */
   SCIP_NODEPRUDATA*     nodeprudata;        /**< node pruner data */
   int                   stdpriority;        /**< priority of the node pruner in standard mode */
   int                   memsavepriority;    /**< priority of the node pruner in memory saving mode */
   SCIP_Bool             initialized;        /**< is node pruner initialized? */
};

#ifdef __cplusplus
}
#endif

#endif
