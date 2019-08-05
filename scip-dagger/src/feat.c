/**@file   feat.c
 * @brief  methods for node features 
 * @author He He 
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <string.h>
#include <stdlib.h>
#include "feat.h"
#include "struct_feat.h"
#include "scip/tree.h"
#include "scip/var.h"
#include "scip/stat.h"
#include "scip/set.h"
#include "scip/visual.h"
#include "scip/struct_scip.h"
#include "helper.h"
#include "math.h"


/* Get number of lines in the file */
static SCIP_Longint getNlines(FILE *infp){
   SCIP_Longint inoffset;
   SCIP_Longint nlines=0;
   char buffer[SCIP_MAXSTRLEN];

   /* Change pointer to the beggining */
   fflush(infp);
   inoffset = ftell(infp);
   rewind(infp);

   /* find out size */
   while( fgets(buffer, (int)sizeof(buffer), infp) != NULL )
      nlines++;
   fseek(infp, inoffset, SEEK_SET);
   return nlines;
}


/** copy feature vector value */
void SCIPfeatCopy(
   SCIP_FEAT*           feat,
   SCIP_FEAT*           sourcefeat 
   )
{
   int i;
   
   assert(feat != NULL);
   assert(feat->vals != NULL);
   assert(sourcefeat != NULL);
   assert(sourcefeat->vals != NULL);

   sourcefeat->maxdepth = feat->maxdepth;
   sourcefeat->depth = feat->depth;
   sourcefeat->size = feat->size;
   sourcefeat->boundtype = feat->boundtype;
   sourcefeat->rootlpobj = feat->rootlpobj;
   sourcefeat->sumobjcoeff = feat->sumobjcoeff;
   sourcefeat->nconstrs = feat->nconstrs;
   sourcefeat->number = feat->number;

   for( i = 0; i < feat->size; i++ )
      sourcefeat->vals[i] = feat->vals[i];
}

/** create feature vector and normalizers, initialized to zero */
SCIP_RETCODE SCIPfeatCreate(
   SCIP*                scip,
   SCIP_FEAT**          feat,
   FILE*                normF,
   int                  size,
   int                  nonprobSize
   )
{
   int i;

   assert(scip != NULL);
   assert(feat != NULL);
   SCIP_CALL( SCIPallocBlockMemory(scip, feat) );

   SCIP_ALLOC( BMSallocMemoryArray(&((*feat)->vals), size) );

   for( i = 0; i < size; i++ )
      (*feat)->vals[i] = 0;
    
   if(normF== NULL){
      (*feat)->norm_max = NULL;
      (*feat)->norm_min = NULL;
      (*feat)->norm_len = 0;
   }
   else{
      int len = getNlines(normF);
      int minlen = (size*2);
      char buffer[SCIP_MAXSTRLEN];
      (*feat)->norm_len = (minlen > len)? minlen:len;
      SCIP_ALLOC( BMSallocMemoryArray(&((*feat)->norm_max), (*feat)->norm_len) );
      SCIP_ALLOC( BMSallocMemoryArray(&((*feat)->norm_min), (*feat)->norm_len) );
      for(i=0; i<len; i++)
      {
        fgets(buffer, (int) sizeof(buffer), normF);
        sscanf(buffer, "%lf,%lf\n", &(((*feat)->norm_min)[i]), &(((*feat)->norm_max)[i]));
      }
      for(i=len; i<(*feat)->norm_len; i++){
        ((*feat)->norm_min)[i] = 0;
        ((*feat)->norm_max)[i] = 0;
      }
   }

   (*feat)->rootlpobj = 0;
   (*feat)->sumobjcoeff = 0;
   (*feat)->nconstrs = 0;
   (*feat)->maxdepth = 0;
   (*feat)->depth = 0;
   (*feat)->size = size;
   (*feat)->nonprobSize = nonprobSize;
   (*feat)->boundtype = 0;
   (*feat)->number = -1;

   return SCIP_OKAY;
}

/** free feature vector */
SCIP_RETCODE SCIPfeatFree(
   SCIP*                scip,
   SCIP_FEAT**          feat 
   )
{
   assert(scip != NULL);
   assert(feat != NULL);
   assert(*feat != NULL);
   BMSfreeMemoryArray(&(*feat)->vals);
   if (((*feat)->norm_max) != NULL)
      BMSfreeMemoryArray(&((*feat)->norm_max));
   if (((*feat)->norm_min) != NULL)
      BMSfreeMemoryArray(&((*feat)->norm_min));
   SCIPfreeBlockMemory(scip, feat);

   return SCIP_OKAY;
}

/** calculate feature values for the node pruner of this node */
void SCIPcalcNodepruFeat(
   SCIP*             scip,
   SCIP_NODE*        node,
   SCIP_FEAT*        feat,
   SCIP_Real*        probfeats,
   int               probfeatssize
   )
{
   SCIP_Real lowerbound;
   SCIP_Real upperbound;
   SCIP_Bool upperboundinf;
   SCIP_Real rootlowerbound;
   SCIP_VAR* branchvar;
   SCIP_BOUNDCHG* boundchgs;
   SCIP_BRANCHDIR branchdirpreferred;
   SCIP_Real branchbound;
   SCIP_Bool haslp;
   SCIP_Real varsol;
   SCIP_Real varrootsol;
   int i, j;

   for( i = 0; i < SCIPfeatGetSize(feat); i++ )
       feat->vals[i] = 0;
       
   assert(node != NULL);
   assert(SCIPnodeGetDepth(node) != 0);
   assert(feat != NULL);
   assert(feat->maxdepth != 0);

   boundchgs = ((node->domchg)->domchgbound).boundchgs;
   assert(boundchgs != NULL);
   assert(boundchgs[0].boundchgtype == SCIP_BOUNDCHGTYPE_BRANCHING);

   feat->depth = SCIPnodeGetDepth(node);

   lowerbound = SCIPgetLowerbound(scip);
   assert(!SCIPsetIsInfinity(scip->set, lowerbound));

   upperbound = SCIPgetUpperbound(scip);
   if( SCIPsetIsInfinity(scip->set, upperbound)
      || SCIPsetIsInfinity(scip->set, -upperbound) )
      upperboundinf = TRUE;
   else
      upperboundinf = FALSE;

   rootlowerbound = REALABS(scip->stat->rootlowerbound);
   if( SCIPsetIsZero(scip->set, rootlowerbound) )
      rootlowerbound = 0.0001;
   assert(!SCIPsetIsInfinity(scip->set, rootlowerbound));

   /* currently only support branching on one variable */
   branchvar = boundchgs[0].var; 
   branchbound = boundchgs[0].newbound;
   branchdirpreferred = SCIPvarGetBranchDirection(branchvar);

   haslp = SCIPtreeHasFocusNodeLP(scip->tree);
   varsol = SCIPvarGetSol(branchvar, haslp);
   varrootsol = SCIPvarGetRootSol(branchvar);

   feat->boundtype = boundchgs[0].boundtype;

   /* calculate features */
   /* global features */
   if( SCIPsetIsEQ(scip->set, upperbound, lowerbound) )
      feat->vals[SCIP_FEATNODEPRU_GAP] = 0;
   else if( SCIPsetIsZero(scip->set, lowerbound)
      || upperboundinf ) 
      feat->vals[SCIP_FEATNODEPRU_GAPINF] = 1;
   else
      feat->vals[SCIP_FEATNODEPRU_GAP] = (upperbound - lowerbound)/REALABS(lowerbound);

   feat->vals[SCIP_FEATNODEPRU_GLOBALLOWERBOUND] = lowerbound / rootlowerbound;
   if( upperboundinf )
      feat->vals[SCIP_FEATNODEPRU_GLOBALUPPERBOUNDINF] = 1;
   else
      feat->vals[SCIP_FEATNODEPRU_GLOBALUPPERBOUND] = upperbound / rootlowerbound;

   feat->vals[SCIP_FEATNODEPRU_NSOLUTION] = SCIPgetNSolsFound(scip);
   feat->vals[SCIP_FEATNODEPRU_PLUNGEDEPTH] = SCIPgetPlungeDepth(scip);
   feat->vals[SCIP_FEATNODEPRU_RELATIVEDEPTH] = (SCIP_Real)feat->depth / (SCIP_Real)feat->maxdepth * 10.0;

   /* node features */
   if( upperboundinf )
      upperbound = lowerbound + 0.2 * (upperbound - lowerbound);
   if( !SCIPsetIsEQ(scip->set, upperbound, lowerbound) )
   {
      feat->vals[SCIP_FEATNODEPRU_RELATIVEBOUND] = (SCIPnodeGetLowerbound(node) - lowerbound) / (upperbound - lowerbound);
      feat->vals[SCIP_FEATNODEPRU_RELATIVEESTIMATE] = (SCIPnodeGetEstimate(node) - lowerbound)/ (upperbound - lowerbound);
   }

   /* branch var features */
   feat->vals[SCIP_FEATNODEPRU_BRANCHVAR_BOUNDLPDIFF] = branchbound - varsol;
   feat->vals[SCIP_FEATNODEPRU_BRANCHVAR_ROOTLPDIFF] = varrootsol - varsol;

   if( branchdirpreferred == SCIP_BRANCHDIR_DOWNWARDS )
      feat->vals[SCIP_FEATNODEPRU_BRANCHVAR_PRIO_DOWN] = 1;
   else if(branchdirpreferred == SCIP_BRANCHDIR_UPWARDS ) 
      feat->vals[SCIP_FEATNODEPRU_BRANCHVAR_PRIO_UP] = 1;

   feat->vals[SCIP_FEATNODEPRU_BRANCHVAR_PSEUDOCOST] = SCIPvarGetPseudocost(branchvar, scip->stat, branchbound - varsol);
   /*fprintf(stderr, "%d cost: %f, varobj: %f", (int)SCIP_FEATNODEPRU_BRANCHVAR_PSEUDOCOST, SCIPvarGetPseudocost(branchvar, scip->stat, branchbound - varsol), varobj);*/

   feat->vals[SCIP_FEATNODEPRU_BRANCHVAR_INF] = 
      feat->boundtype == SCIP_BOUNDTYPE_LOWER ? 
      SCIPvarGetAvgInferences(branchvar, scip->stat, SCIP_BRANCHDIR_UPWARDS) / (SCIP_Real)feat->maxdepth : 
      SCIPvarGetAvgInferences(branchvar, scip->stat, SCIP_BRANCHDIR_DOWNWARDS) / (SCIP_Real)feat->maxdepth;

   //feat->vals[SCIP_FEATNODEPRU_OFFSET] = ceil(SCIPfeatGetLinearOffset(feat));

   if(probfeatssize != 0){
      for(j=0; j<probfeatssize; j++)
        feat->vals[j + SCIP_FEATNODEPRU_SIZE] = probfeats[j];
   }


}

void SCIPtraverseToRoot(
   SCIP*             scip,
   SCIP_NODE*        node,
   SCIP_Real*        nodefeats
){
   SCIP_NODE* cnode=node;
   int i=0, step=0, obst=0, side=0;

   if (SCIPgetNBinVars(scip) != 0){
      if (SCIPgetNContVars(scip) != 0){
        /* Code not tested yet */
         SCIP_Real buffer[NTS][NOBST][4][2]; /* four sides */
         SCIP_Real* tmpbuf = (SCIP_Real*) &buffer;
         for(i=0; i<(NTS*NOBST*8); i++)
            tmpbuf[i] = -1;

         while (SCIPnodeGetNumber(cnode) != 1){
            /* fill node features here */
            step = SCIPnodeGetIndex1(cnode);
            obst = SCIPnodeGetIndex2(cnode);
            side = SCIPnodeGetIndex3(cnode);
            buffer[step][obst][side][0] = SCIPnodeGetBranchBound(cnode);
            buffer[step][obst][side][1] = SCIPnodeGetBoundType(cnode);

            /* next process parent */
            cnode = SCIPnodeGetParent(cnode);
         }
         memcpy(nodefeats, buffer, (NTS*NOBST*8)*sizeof(SCIP_Real));
      }
      else{
         SCIP_Real *buffer;
         int vidx;
         SCIPallocClearMemoryArray(scip, &buffer, SCIPgetNBinVars(scip)); /*lint !e506*/
         for(i=0; i < SCIPgetNBinVars(scip); i++)
            buffer[i] = -1;

         while (SCIPnodeGetNumber(cnode) != 1){
            int boundtype, branchbound;
            vidx = SCIPnodeGetIndex1(cnode);
            branchbound = SCIPnodeGetBranchBound(cnode);
            boundtype = SCIPnodeGetBoundType(cnode);

            if(boundtype == SCIP_BOUNDTYPE_LOWER){
               assert(branchbound != 1);
               buffer[vidx] = 1;
            }
            else if(boundtype == SCIP_BOUNDTYPE_UPPER){
               assert(branchbound != 0);
               buffer[vidx] = 0;
            }

            /* next process parent */
            cnode = SCIPnodeGetParent(cnode);
         }
         memcpy(nodefeats, buffer, SCIPgetNBinVars(scip)*sizeof(SCIP_Real));
      }
   }
   else{
      #define MAX_TMP 1000

      SCIP_Real buffer[NTS][2][2]; /* 2 - dimensional; 2 - upper/lower */
      SCIP_Real* tmpbuf = (SCIP_Real*) &buffer;
      for(i=0; i<(NTS*2)*2; i++){
        tmpbuf[i] = MAX_TMP;
      }

      while (SCIPnodeGetNumber(cnode) != 1){
        /* next process parent */
        step = SCIPnodeGetIndex1(cnode);
        side = SCIPnodeGetIndex2(cnode);
        if (buffer[step][side][0] >= MAX_TMP)
          buffer[step][side][0] = SCIPnodeGetLb(cnode);
        else
          buffer[step][side][0] = max(SCIPnodeGetLb(cnode), buffer[step][side][0]);

        if (buffer[step][side][1] >= MAX_TMP)
          buffer[step][side][1] = SCIPnodeGetUb(cnode);
        else
          buffer[step][side][1] = min(SCIPnodeGetLb(cnode), buffer[step][side][1]);
        cnode = SCIPnodeGetParent(cnode);
      }

      for(i=0; i<(NTS*2)*2; i++){
        if (tmpbuf[i] >= MAX_TMP)
          nodefeats[i] = 0;
        else
          nodefeats[i] = tmpbuf[i];
      }
   }
}

/** calculate feature values for the node selector of this node */
void SCIPcalcNodeselFeat(
   SCIP*             scip,
   SCIP_NODE*        node,
   SCIP_FEAT*        feat,
   SCIP_Real*        probfeats,
   int               probfeatssize
   )
{
   SCIP_NODETYPE nodetype;
   SCIP_Real nodelowerbound;
   SCIP_Real rootlowerbound;
   SCIP_Real lowerbound;            /**< global lower bound */
   SCIP_Real upperbound;           /**< global upper bound */
   SCIP_VAR* branchvar;
   SCIP_BOUNDCHG* boundchgs;
   SCIP_BRANCHDIR branchdirpreferred;
   SCIP_Real branchbound;
   SCIP_Bool haslp;
   SCIP_Bool upperboundinf;
   SCIP_Real varsol;
   SCIP_Real varrootsol;
   int i, j;

   assert(node != NULL);
   assert(SCIPnodeGetDepth(node) != 0);
   assert(feat != NULL);
   assert(feat->maxdepth != 0);

   for(i = 0; i < SCIPfeatGetSize(feat); i++){
       feat->vals[i] = 0;
   }

   boundchgs = ((node->domchg)->domchgbound).boundchgs;
   assert(boundchgs != NULL);
   assert(boundchgs[0].boundchgtype == SCIP_BOUNDCHGTYPE_BRANCHING);

   /* node number used to track feasible solution */
   feat->number = SCIPnodeGetNumber(node);

   /* extract necessary information */
   nodetype = SCIPnodeGetType(node);
   nodelowerbound = SCIPnodeGetLowerbound(node);
   rootlowerbound = REALABS(scip->stat->rootlowerbound);
   if( SCIPsetIsZero(scip->set, rootlowerbound) )
      rootlowerbound = 0.0001;
   assert(!SCIPsetIsInfinity(scip->set, rootlowerbound));
   lowerbound = SCIPgetLowerbound(scip);
   upperbound = SCIPgetUpperbound(scip);
   if( SCIPsetIsInfinity(scip->set, upperbound)
      || SCIPsetIsInfinity(scip->set, -upperbound) )
      upperboundinf = TRUE;
   else
      upperboundinf = FALSE;
   feat->depth = SCIPnodeGetDepth(node);

   if( upperboundinf )
      upperbound = lowerbound + 0.2 * (upperbound - lowerbound);

   /* global features */
  #if 1
    if( SCIPsetIsEQ(scip->set, upperbound, lowerbound) )
       feat->vals[SCIP_FEATNODESEL_GAP] = 0;
    else if( SCIPsetIsZero(scip->set, lowerbound)
       || upperboundinf ) 
       feat->vals[SCIP_FEATNODESEL_GAPINF] = 1;
    else
       feat->vals[SCIP_FEATNODESEL_GAP] = (upperbound - lowerbound)/REALABS(lowerbound);

    if( upperboundinf )
    {
       feat->vals[SCIP_FEATNODESEL_GLOBALUPPERBOUNDINF] = 1;
       /* use only 20% of the gap as upper bound */
       upperbound = lowerbound + 0.2 * (upperbound - lowerbound);
    }
    else
       feat->vals[SCIP_FEATNODESEL_GLOBALUPPERBOUND] = upperbound / rootlowerbound;

   //  feat->vals[SCIP_FEATNODESEL_PLUNGEDEPTH] = SCIPgetPlungeDepth(scip);
   //  feat->vals[SCIP_FEATNODESEL_RELATIVEDEPTH] = (SCIP_Real)feat->depth / (SCIP_Real)feat->maxdepth * 10.0;
  #endif


   /* currently only support branching on one variable */
   branchvar = boundchgs[0].var; 
   branchbound = boundchgs[0].newbound;
   branchdirpreferred = SCIPvarGetBranchDirection(branchvar);

   haslp = SCIPtreeHasFocusNodeLP(scip->tree);
   varsol = SCIPvarGetSol(branchvar, haslp);
   varrootsol = SCIPvarGetRootSol(branchvar);

   feat->boundtype = boundchgs[0].boundtype;

#if 1
   /* calculate features */
   feat->vals[SCIP_FEATNODESEL_LOWERBOUND] = 
      nodelowerbound / rootlowerbound;

   feat->vals[SCIP_FEATNODESEL_ESTIMATE] = 
      SCIPnodeGetEstimate(node) / rootlowerbound;

   if( !SCIPsetIsEQ(scip->set, upperbound, lowerbound) )
      feat->vals[SCIP_FEATNODESEL_RELATIVEBOUND] = (nodelowerbound - lowerbound) / (upperbound - lowerbound);

   if( nodetype == SCIP_NODETYPE_SIBLING )
      feat->vals[SCIP_FEATNODESEL_TYPE_SIBLING] = 1;
   else if( nodetype == SCIP_NODETYPE_CHILD )
      feat->vals[SCIP_FEATNODESEL_TYPE_CHILD] = 1;
   else if( nodetype == SCIP_NODETYPE_LEAF )
      feat->vals[SCIP_FEATNODESEL_TYPE_LEAF] = 1;

   feat->vals[SCIP_FEATNODESEL_BRANCHVAR_BOUNDLPDIFF] = branchbound - varsol;
   feat->vals[SCIP_FEATNODESEL_BRANCHVAR_ROOTLPDIFF] = varrootsol - varsol;

   if( branchdirpreferred == SCIP_BRANCHDIR_DOWNWARDS )
      feat->vals[SCIP_FEATNODESEL_BRANCHVAR_PRIO_DOWN] = 1;
   else if(branchdirpreferred == SCIP_BRANCHDIR_UPWARDS ) 
      feat->vals[SCIP_FEATNODESEL_BRANCHVAR_PRIO_UP] = 1;

   feat->vals[SCIP_FEATNODESEL_BRANCHVAR_PSEUDOCOST] = SCIPvarGetPseudocost(branchvar, scip->stat, branchbound - varsol);

   feat->vals[SCIP_FEATNODESEL_BRANCHVAR_INF] = 
      feat->boundtype == SCIP_BOUNDTYPE_LOWER ? 
      SCIPvarGetAvgInferences(branchvar, scip->stat, SCIP_BRANCHDIR_UPWARDS) / (SCIP_Real)feat->maxdepth : 
      SCIPvarGetAvgInferences(branchvar, scip->stat, SCIP_BRANCHDIR_DOWNWARDS) / (SCIP_Real)feat->maxdepth;

   //feat->vals[SCIP_FEATNODESEL_OFFSET] = SCIPfeatGetOffset(feat);
#endif

   if(probfeatssize != 0){
      for(j=0; j<probfeatssize; j++){
        feat->vals[j + SCIP_FEATNODESEL_SIZE] = probfeats[j];
      }
   }

   if(BUFF != 0){
      int offset = SCIP_FEATNODESEL_SIZE + probfeatssize;
      SCIPtraverseToRoot(scip, node, feat->vals + offset);
   }

   return;
}

/** write feature vector diff (feat1 - feat2) in libsvm format */
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
   )
{
   int size;
   int i;
   int offset1;
   int offset2;
   SCIP_Real weight;

   assert(scip != NULL);
   assert(feat1 != NULL);
   assert(feat2 != NULL);
   assert(feat1->depth != 0);
   assert(feat2->depth != 0);
   assert(feat1->size == feat2->size);

   if(label == 1){
     weight = SCIPfeatGetWeight(feat1);
     SCIPinfoMessage(scip, wfile, "%f\n", scale * weight);  
   }
   else{
     weight = SCIPfeatGetWeight(feat2);
     SCIPinfoMessage(scip, wfile, "%f\n", scale * weight);  
   }

   if( negate )
   {
      SCIP_FEAT* tmp = feat1;
      feat1 = feat2;
      feat2 = tmp;
      label = -1 * label;
   }

   size = SCIPfeatGetSize(feat1);
   offset1 = SCIPfeatGetOffset(feat1);
   offset2 = SCIPfeatGetOffset(feat2);
 
#ifdef LIBLINEAR

   SCIPinfoMessage(scip, file1, "%d ", label);  
   assert(file2 == NULL);

   if( offset1 == offset2 )
   {
      for( i = 0; i < size; i++ )
         SCIPinfoMessage(scip, file1, "%d:%f ", i + offset1 + 1, feat1->vals[i] - feat2->vals[i]);
   }
   else
   {
      /* libsvm requires sorted indices, write smaller indices first */
      if( offset1 < offset2 )
      {
         /* feat1 */
         for( i = 0; i < size; i++ )
            SCIPinfoMessage(scip, file1, "%d:%f ", i + offset1 + 1, feat1->vals[i]);
         /* -feat2 */
         for( i = 0; i < size; i++ )
            SCIPinfoMessage(scip, file1, "%d:%f ", i + offset2 + 1, -feat2->vals[i]);
      }
      else
      {
         /* -feat2 */
         for( i = 0; i < size; i++ )
            SCIPinfoMessage(scip, file1, "%d:%f ", i + offset2 + 1, -feat2->vals[i]);
         /* feat1 */
         for( i = 0; i < size; i++ )
            SCIPinfoMessage(scip, file1, "%d:%f ", i + offset1 + 1, feat1->vals[i]);
      }
   }

   SCIPinfoMessage(scip, file1, "\n");

#else

   if(label > 0){
      SCIPinfoMessage(scip, file1, "1 ");  
      SCIPinfoMessage(scip, file2, "0 ");  
      for( i = 0; i < size; i++ ){
        SCIPinfoMessage(scip, file1, "%d:%.4f ", i + offset1 + 1, feat1->vals[i]);
        SCIPinfoMessage(scip, file2, "%d:%.4f ", i + offset2 + 1, feat2->vals[i]);
      }
   }
   else{
      SCIPinfoMessage(scip, file1, "1 ");  
      SCIPinfoMessage(scip, file2, "0 ");  
      for( i = 0; i < size; i++ ){
        SCIPinfoMessage(scip, file1, "%d:%.4f ", i + offset2 + 1, feat2->vals[i]);
        SCIPinfoMessage(scip, file2, "%d:%.4f ", i + offset1 + 1, feat1->vals[i]);
      }
   }
   SCIPinfoMessage(scip, file1, "\n");
   SCIPinfoMessage(scip, file2, "\n");

#endif

   return;
}

/* Write the Features vector with node number to a file */
void SCIPfeatPrint(
   SCIP*             scip,
   FILE*             file,
   SCIP_FEAT*        feat,
   SCIP_Longint      nodenumber,
   SCIP_Longint      parentid,
   int               label
   )
{
   int size;
   int i;
   int offset;
   SCIP_Real weight;

   assert(scip != NULL);
   assert(file != NULL);
   assert(feat != NULL);
   assert(feat->depth != 0);

   weight = SCIPfeatGetWeight(feat);
   size = SCIPfeatGetSize(feat);
   offset = SCIPfeatGetOffset(feat);

   SCIPinfoMessage(scip, file, "%f ", weight);  
   SCIPinfoMessage(scip, file, "%d ", nodenumber);  
   SCIPinfoMessage(scip, file, "%d ", parentid);  
   SCIPinfoMessage(scip, file, "%d ", label);  
   SCIPinfoMessage(scip, file, "%d ", offset);
   for( i = 0; i < size; i++ )
      SCIPinfoMessage(scip, file, "%f ", feat->vals[i]);

   SCIPinfoMessage(scip, file, "\n");
   return;
}

/* Write the Compare Features vector */
void SCIPfeatcmpPrint(
   SCIP*             scip,
   FILE*             file,
   SCIP_FEAT*        feat,
   SCIP_Longint      nodenumber
   )
{
   int size;
   int i;
   int offset;
   SCIP_Real weight;

   assert(scip != NULL);
   assert(file != NULL);
   assert(feat != NULL);
   assert(feat->depth != 0);

   weight = SCIPfeatGetWeight(feat);
   size = SCIPfeatGetSize(feat);
   offset = SCIPfeatGetOffset(feat);

   SCIPinfoMessage(scip, file, "%f ", weight);  
   SCIPinfoMessage(scip, file, "%d ", nodenumber);  
   SCIPinfoMessage(scip, file, "%d ", offset);  
   for( i = 0; i < size; i++ )
      SCIPinfoMessage(scip, file, "%f ", feat->vals[i]);

   SCIPinfoMessage(scip, file, "\n");
   return;
}

/** write feature vector in libsvm format */
void SCIPfeatLIBSVMPrint(
   SCIP*             scip,
   FILE*             file,
   FILE*             wfile,
   SCIP_FEAT*        feat,
   float             scale,
   int               label
   )
{
   int size;
   int i;
   int offset;
   SCIP_Real weight;

   assert(scip != NULL);
   assert(feat != NULL);
   assert(feat->depth != 0);

   weight = SCIPfeatGetWeight(feat);
   SCIPinfoMessage(scip, wfile, "%f\n", scale * weight);  

   size = SCIPfeatGetSize(feat);
   offset = SCIPfeatGetOffset(feat);

   SCIPinfoMessage(scip, file, "%d ", label);  

   for( i = 0; i < size; i++ )
      SCIPinfoMessage(scip, file, "%d:%f ", i + offset + 1, feat->vals[i]);

   SCIPinfoMessage(scip, file, "\n");
}

/* Read the problem specific features */
void readProbFeats(
    FILE           *file, 
    SCIP_Real      *feats,
    int             size
)
{
    int i=0;
    for(i=0; i<size; i++){
        fscanf(file, "%lf\n", &feats[i]);
    }
    return;
}

/*
 * simple functions implemented as defines
 */

/* In debug mode, the following methods are implemented as function calls to ensure
 * type validity.
 * In optimized mode, the methods are implemented as defines to improve performance.
 * However, we want to have them in the library anyways, so we have to undef the defines.
 */

#undef SCIPfeatGetSize
#undef SCIPfeatGetVals
#undef SCIPfeatGetOffset
#undef SCIPfeatGetLinearOffset
#undef SCIPfeatSetRootlpObj
#undef SCIPfeatSetSumObjCoeff
#undef SCIPfeatSetMaxDepth
#undef SCIPfeatSetNConstrs

void SCIPfeatSetRootlpObj(
   SCIP_FEAT*    feat,
   SCIP_Real     rootlpobj
   )
{
   assert(feat != NULL);

   feat->rootlpobj = rootlpobj;
}

SCIP_Real* SCIPfeatGetVals(
   SCIP_FEAT*    feat 
   )
{
   assert(feat != NULL);

   return feat->vals;
}

int SCIPfeatGetSize(
   SCIP_FEAT*    feat 
   )
{
   assert(feat != NULL);
   return feat->size;
}

SCIP_Longint SCIPfeatGetDepth(
   SCIP_FEAT*    feat 
   )
{
  assert(feat != NULL);
  return feat->depth;
}

void SCIPfeatSetSumObjCoeff(
   SCIP_FEAT*    feat,
   SCIP_Real     sumobjcoeff
   )
{
   assert(feat != NULL);

   feat->sumobjcoeff = sumobjcoeff;
}

void SCIPfeatSetMaxDepth(
   SCIP_FEAT*    feat,
   int           maxdepth
   )
{
   assert(feat != NULL);

   //feat->maxdepth = maxdepth;
   feat->maxdepth = 2000;
}

void SCIPfeatSetNConstrs(
   SCIP_FEAT*    feat,
   int           nconstrs 
   )
{
   assert(feat != NULL);

   feat->nconstrs = nconstrs;
}

/** returns the weight of the example */
SCIP_Real SCIPfeatGetWeight(
   SCIP_FEAT* feat
   )
{
   assert(feat != NULL);
   /* depth=1 => weight = 5; depth=0.6*maxdepth => weight = 1 */
   //return 20*exp(-(feat->depth)/25);
   return 1;
}

SCIP_Longint SCIPfeatGetNumber(
   SCIP_FEAT* feat
)
{
   assert(feat != NULL);
   return feat->number;
}

int SCIPfeatGetOffset(
   SCIP_FEAT* feat
   )
{
   assert(feat != NULL);
#ifdef LIBLINEAR
   return (feat->size * 2) * (feat->depth / (feat->maxdepth / 10)) + (feat->size * (int)feat->boundtype);
#else
   //return (feat->size * (int)feat->boundtype);
   return 0;
#endif
}

int SCIPfeatGetLinearOffset(
    SCIP_FEAT* feat
)
{
    assert(feat != NULL);
    return feat->depth;
    //return 5*exp(((1-feat->depth)*1.61)/(0.01*feat->maxdepth));
    //return (feat->depth / (feat->maxdepth / 25));
}
