#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "scip/type_relax.h"
#include "scip/sol.h"
#include "scip/def.h"

#ifndef HELPER_H
#define HELPER_H

EXTERN
int countnFeasible(
    char* prefix
);

EXTERN
SCIP_Real getBestObj(
    SCIP_SOL**      allsols, 
    int             nsols
);

EXTERN
SCIP_Longint getSolIndex(
    SCIP*           scip,
    SCIP_NODE*      node, 
    SCIP_SOL**      allsols, 
    SCIP_Bool*      solflag,
    int             nsols,
    SCIP_Real       margin
);

void getSolFname(
    char*      fname, 
    char*      prefix, 
    int        count
);

SCIP_Bool SCIPnodeSetFeats(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE*            node
   );


void setNodeUbLb(SCIP_NODE* node);

#define NTS       15 /* Number of time steps */
#define NOBST     18 /* Number of obstacles  */

//#define BUFF      (SCIPgetNBinVars(scip)?(NTS*NOBST)*16:(NTS*2)*2) // For pSulu
//#define BUFF      SCIPgetNBinVars(scip)  /* No margins for now -- if you have only binary variables */

#define BUFF      0

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif
   
#ifndef min
    #define min(a,b) ((a) < (b) ? (a) : (b))
#endif
 
#endif
