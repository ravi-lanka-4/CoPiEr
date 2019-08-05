/**@file   policy.h
 * @brief  internal methods for node policyures 
 * @author He He 
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_POLICY_H__
#define __SCIP_POLICY_H__

#include "scip/def.h"
#include "scip/scip.h"
#include "struct_policy.h"

#ifdef __cplusplus
extern "C" {
#endif

float getBinomial();
float getRandom(float stddev);

#ifdef LIBLINEAR

extern 
SCIP_RETCODE SCIPpolicyCreate(
   SCIP*              scip,
   SCIP_POLICY**      policy
   );

extern 
SCIP_RETCODE SCIPpolicyFree(
   SCIP*              scip,
   SCIP_POLICY**      policy
   );

  
/** read policy (model) in LIBSVM format */
extern
SCIP_RETCODE SCIPreadLIBSVMPolicy(
   SCIP*              scip,
   char*              fname,
   SCIP_POLICY**      policy
   );
   
/** calculate score of a node given its feature and the policy weight vector */
extern
void SCIPcalcNodeScore(
   SCIP_NODE*         node,
   SCIP_FEAT*         feat,
   SCIP_POLICY*       policy,
   int                probfeatsize
   );

#else

extern
SCIP_RETCODE SCIPkerasPolicyCreate(
   SCIP*              scip,
   void**             policy
   );

extern 
SCIP_RETCODE SCIPkerasPolicyFree(
   SCIP*              scip,
   void**             policy
   );

extern
void SCIPcalcKerasNodeScore(
   SCIP_NODE*         node,
   SCIP_FEAT*         feat,
   void**             policy,
   int                probfeatsize
   );

extern
SCIP_RETCODE SCIPreadKerasPolicy(
   SCIP*              scip,
   char*              fname,
   void**             policy       
   );

#endif

#ifdef __cplusplus
}
#endif

#endif
