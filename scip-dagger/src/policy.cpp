/**@file   policy.c
 * @brief  methods for policy
 * @author He He 
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <random>
#include "scip/def.h"
#include "feat.h"
#include "keras_model.h"
#include "struct_feat.h"
#include "policy.h"
#include <iostream>

#define HEADERSIZE_LIBSVM       6 
using namespace std;

static std::default_random_engine generator(time(0));
static std::default_random_engine generator_bin(time(0));

float getBinomial(){
  std::uniform_real_distribution<double> distribution(0.0, 1.0);
  return distribution(generator_bin);
}

float getRandom(float stddev){
  std::normal_distribution<double> distribution(0, stddev);
  return distribution(generator);
}

#ifdef LIBLINEAR

SCIP_RETCODE SCIPpolicyCreate(
   SCIP*              scip,
   SCIP_POLICY**      policy
   )
{
   assert(scip != NULL);
   assert(policy != NULL);

   SCIP_CALL( SCIPallocBlockMemory(scip, policy) );
   (*policy)->weights = NULL;
   (*policy)->size = 0;
   return SCIP_OKAY;
}

SCIP_RETCODE SCIPpolicyFree(
   SCIP*              scip,
   SCIP_POLICY**      policy
   )
{
   assert(scip != NULL);
   assert(policy != NULL);
   assert((*policy)->weights != NULL);

   BMSfreeMemoryArray(&(*policy)->weights);
   SCIPfreeBlockMemory(scip, policy);

   return SCIP_OKAY;
}

SCIP_RETCODE SCIPreadLIBSVMPolicy(
   SCIP*              scip,
   char*              fname,
   SCIP_POLICY**      policy       
   )
{
   int nlines = 0;
   int i;
   char buffer[SCIP_MAXSTRLEN];

   FILE* file = fopen(fname, "r");
   if( file == NULL )
   {
      SCIPerrorMessage("cannot open file <%s> for reading\n", fname);
      SCIPprintSysError(fname);
      return SCIP_NOFILE;
   }

   /* find out weight vector size */
   while( fgets(buffer, (int)sizeof(buffer), file) != NULL )
      nlines++;
   /* don't count libsvm model header */
   assert(nlines >= HEADERSIZE_LIBSVM);
   (*policy)->size = nlines - HEADERSIZE_LIBSVM;
   fclose(file);
   if( (*policy)->size == 0 )
   {
      SCIPerrorMessage("empty policy model\n");
      return SCIP_NOFILE;
   }

   SCIP_CALL( SCIPallocMemoryArray(scip, &(*policy)->weights, (*policy)->size) );

   /* have to reopen to read weights */
   file = fopen(fname, "r");
   /* skip header */
   for( i = 0; i < HEADERSIZE_LIBSVM; i++ )
      fgets(buffer, (int)sizeof(buffer), file);
   for( i = 0; i < (*policy)->size; i++ )
      fscanf(file, "%" SCIP_REAL_FORMAT, &((*policy)->weights[i]));

   fclose(file);

   SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "policy of size %d from file <%s> was %s\n",
      (*policy)->size, fname, "read, will be used in the dagger node selector");

   return SCIP_OKAY;
}

/** calculate score of a node given its feature and the policy weight vector */
void SCIPcalcNodeScore(
   SCIP_NODE*         node,
   SCIP_FEAT*         feat,
   SCIP_POLICY*       policy,
   int                probfeatsize
   )
{
   int offset = SCIPfeatGetOffset(feat);
   int i;
   SCIP_Real score = 0;
   SCIP_Real* weights = policy->weights;
   SCIP_Real* featvals = SCIPfeatGetVals(feat);
   SCIP_Real normdenom;

   /* Not support for now and needs to be fixed */
   #error

   if( (offset + SCIPfeatGetSize(feat)) > policy->size ){
       int seg = policy->size/(feat->size * 2);
       offset = (feat->size * 2) * (seg-1) + (feat->size * (int)feat->boundtype);
   }

   if ((feat->norm_max != NULL) && (feat->norm_min != NULL)){
      /* Normalize data to [-1, 1] (don't normalize problem features) */
      for( i = 0; i < SCIPfeatGetSize(feat); i++ ){
         int j = i + offset;
         normdenom = (feat->norm_max[j]-feat->norm_min[j]);
         /* In case feature's max and min are the same, divide by 1 instead of 0 */
         normdenom = (normdenom != 0) ? normdenom : 1;
         if (i < SCIPfeatGetSize(feat) - probfeatsize) {
             score += ((weights[i+offset] * ((2 * (featvals[i] - feat->norm_min[j]))/normdenom - 1)));
         }
         else {
             score += featvals[i] * weights[i+offset];
         }
      }
   }
   else{
      /* No Normalization */
      for( i = 0; i < SCIPfeatGetSize(feat); i++ ){
         score += featvals[i] * weights[i+offset];
      }
   }

   SCIPnodeSetScore(node, score);
   SCIPdebugMessage("score of node  #%" SCIP_LONGINT_FORMAT ": %f\n", SCIPnodeGetNumber(node), SCIPnodeGetScore(node));
}

#else

SCIP_RETCODE SCIPkerasPolicyCreate(
   SCIP*              scip,
   void**             policy
   )
{
   assert(scip != NULL);
   assert(policy != NULL);
   KerasModel* model = new KerasModel();

   *policy = (void *) model;
   return SCIP_OKAY;
}

SCIP_RETCODE SCIPkerasPolicyFree(
   SCIP*      scip,
   void**     policy
   )
{
   assert(scip != NULL);
   assert(policy != NULL);

   KerasModel* model = (KerasModel *) *policy;
   delete model;

   return SCIP_OKAY;
}

SCIP_RETCODE SCIPreadKerasPolicy(
   SCIP*              scip,
   char*              fname,
   void**             policy       
   )
{
   KerasModel *model = (KerasModel *) *policy;
   FILE* file = fopen(fname, "r");
   if( file == NULL )
   {
      SCIPerrorMessage("cannot open file <%s> for reading\n", fname);
      SCIPprintSysError(fname);
      return SCIP_NOFILE;
   }

   model->LoadModel(fname);
   return SCIP_OKAY;
}

/** calculate score of a node given its feature and the policy weight vector */
void SCIPcalcKerasNodeScore(
   SCIP_NODE*         node,
   SCIP_FEAT*         feat,
   void**             policy,
   int                probfeatsize
   )
{
   int offset = SCIPfeatGetOffset(feat);
   KerasModel *model = (KerasModel *) *policy;

   // Create a 1D Tensor on length 10 for input data.
   Tensor in(feat->size);
   in.Fill(0);
   SCIP_Real normdenom;

   // Without Normalization
   assert(offset == 0);
   for(int i=0; i<feat->size; i++){
      in.data_[i + offset] = feat->vals[i];
   }

   if ((feat->norm_max != NULL) && (feat->norm_min != NULL)){
      // With normalization to [-1, 1] and don't normalize problem features
      for(int i=0; i<(feat->size); i++){
        normdenom = (feat->norm_max[i]-feat->norm_min[i]);
        /* In case feature's max and min are the same, divide by 1 instead of 0 */
        normdenom = (normdenom != 0) ? normdenom : 1;
        in.data_[i] = ((2 * (in.data_[i] - feat->norm_min[i]))/normdenom) - 1;
      }
   }

   Tensor out(1);
   out.data_[0] = 0;
   model->Apply(&in, &out);

#if 0
   if (feat->size == 16){
    std::cout << "Size: " << feat->size << std::endl;
    for(int i=0; i<2*(feat->size); i++){
      std::cout << in.data_[i] << " ";
    } 
    std::cout << "Score: " << out.data_[0] << std::endl;
  }
#endif

   // Save socre in the node
   SCIPnodeSetScore(node, out.data_[0]);
   return;
}

#endif
