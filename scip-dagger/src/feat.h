/**@file   feat.h
 * @brief  internal methods for node features 
 * @author He He 
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_FEAT_H__
#define __SCIP_FEAT_H__

#include "scip/def.h"
#include "scip/scip.h"
#include "scip/type_lp.h"
#include "pub_feat.h"

#ifdef NDEBUG
#include "struct_feat.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** write feature vector in libsvm format */
extern
void SCIPfeatLIBSVMPrint(
   SCIP*             scip,
   FILE*             file,
   FILE*             wfile,
   SCIP_FEAT*        feat,
   float             scale,
   int               label
   );

/* Write the nodenumber of comparisons to a file */
extern
void SCIPcmpPrint(
   SCIP*             scip,
   FILE*             file,
   SCIP_Longint      optnumber,
   SCIP_Longint      nodenumber,
   SCIP_Longint      parentnodenum
);

/** write feature vector diff (feat1 - feat2) in libsvm format */
extern
void SCIPfeatDiffLIBSVMPrint(
   SCIP*             scip,
   FILE*             file1,
   FILE*             file2,
   FILE*             wfile,
   SCIP_FEAT*        feat1,
   SCIP_FEAT*        feat2,
   int               label,
   float             scale,
   SCIP_Bool         negate
   );

/** calculate feature values for the node pruner of this node */
extern
void SCIPcalcNodepruFeat(
   SCIP*             scip,
   SCIP_NODE*        node,
   SCIP_FEAT*        feat,
   SCIP_Real*        probfeats,
   int               probfeatssize
   );

/** calculate feature values for the node selector of this node */
extern
void SCIPcalcNodeselFeat(
   SCIP*             scip,
   SCIP_NODE*        node,
   SCIP_FEAT*        feat,
   SCIP_Real*        probfeats,
   int               probfeatssize
   );

/** returns offset of the feature index */
extern
int SCIPfeatGetOffset(
   SCIP_FEAT* feat
   );

/** returns offset of the linear feature index */
extern
int SCIPfeatGetLinearOffset(
   SCIP_FEAT* feat
   );

/** returns the weight of the example */
extern
SCIP_Real SCIPfeatGetWeight(
   SCIP_FEAT* feat
   );

/** create feature vector and normalizers, initialized to zero */
extern
SCIP_RETCODE SCIPfeatCreate(
   SCIP*                scip,
   SCIP_FEAT**          feat,
   FILE*                normfname,
   int                  size,
   int                  nonprobSize
   );

/** copy feature vector value */
extern
void SCIPfeatCopy(
   SCIP_FEAT*           feat,
   SCIP_FEAT*           sourcefeat 
   );

/** free feature vector */
extern
SCIP_RETCODE SCIPfeatFree(
   SCIP*                scip,
   SCIP_FEAT**          feat 
   );

/** write feature vector in libsvm format */
extern
void SCIPfeatPrint(
   SCIP*             scip,
   FILE*             file,
   SCIP_FEAT*        feat,
   SCIP_Longint      nodenumber,
   SCIP_Longint      parentid,
   int               label
);

/* Write the Compare Features vector */
extern
void SCIPfeatcmpPrint(
   SCIP*             scip,
   FILE*             file,
   SCIP_FEAT*        feat,
   SCIP_Longint      nodenumber
   );

extern
SCIP_Longint SCIPfeatGetNumber(
    SCIP_FEAT*       feat
    );

extern
SCIP_Longint SCIPfeatGetDepth(
    SCIP_FEAT*       feat
    );

/* In optimized mode, the function calls are overwritten by defines to reduce the number of function calls and
 * speed up the algorithms.
 */

#ifdef LIBLINEAR
    #define SCIPfeatGetOffset(feat) (feat->size * 2) * (feat->depth / (feat->maxdepth / 10)) + (feat->size * (int)feat->boundtype);
#else
    #define SCIPfeatGetOffset(feat) 0 //(feat->size * (int)feat->boundtype);
#endif

//#define SCIPfeatGetLinearOffset(feat) (feat->depth / (feat->maxdepth / 25));

//#define SCIPfeatGetLinearOffset(feat)  (5*exp(((1-feat->depth)*1.61)/(0.01*feat->maxdepth)))
#define SCIPfeatGetLinearOffset(feat)  feat->depth

#ifdef __cplusplus
}
#endif

#endif
