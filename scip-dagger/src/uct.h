#ifndef UCT_H
#define UCT_H

/** get the number times @p node has been visited so far */
static inline int nodeGetVisits(
   SCIP_NODESELDATA*     nodeseldata,        /**< node selector data */
   SCIP_NODE*            node                /**< the node in question */
   )
{
   int nodenumber;

   assert(nodeseldata != NULL);
   assert(node != NULL);

   /* nodes numbers start with 1 for the root node */
   nodenumber = (int)(SCIPnodeGetNumber(node) - 1);
   assert(nodenumber >= 0);

   return nodeseldata->nodevisits[nodenumber];
}

/** increases the visits counter along the path from @p node to the root node */
static inline void updateVisits(
   SCIP_NODESELDATA*     nodeseldata,        /**< node selector data */
   SCIP_NODE*            node                /**< leaf node of path along which the visits are backpropagated */
   )
{
   int* visits;

   assert(nodeseldata != NULL);
   assert((void*) node != NULL);

   visits = nodeseldata->nodevisits;
   assert(visits != NULL);

   /* increase visits counter of all nodes along the path until root node is reached (which has NULL as parent) */
   do
   {
      int nodenumber;

      nodenumber = (int)(SCIPnodeGetNumber(node) - 1);
      if( nodenumber < nodeseldata->sizenodevisits )
         ++(visits[nodenumber]);

      assert(SCIPnodeGetParent(node) == NULL || SCIPnodeGetDepth(node) >= 1);
      node = SCIPnodeGetParent(node);
   }
   while( node != NULL );
}


/** returns UCT score of @p node; the UCT score is a mixture of the node's lower bound or estimate and the number of times
 *  it has been visited so far in relation with the number of times its parent has been visited so far
 */
static inline SCIP_Real nodeGetUctScore(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE*            node,               /**< the node for which UCT score is requested */
   SCIP_NODESELDATA*     nodeseldata         /**< node selector data */
   )
{
   SCIP_NODE* parent;
   SCIP_Real rootlowerbound;
   SCIP_Real score;
   int parentvisits;

   /* the visits part of the UCT score function */
   parent = SCIPnodeGetParent(node);
   assert(parent != NULL);
   parentvisits = nodeGetVisits(nodeseldata, parent);

   if(parentvisits > 0)
   {
      int visits;

      visits = nodeGetVisits(nodeseldata, node);
      score = nodeseldata->weight * (parentvisits / (SCIP_Real)(1 + visits));
   }
   else
   {
      score = 0;
   }

   return score;
}

/** compares two leaf nodes by comparing the UCT scores of the two children of their deepest common ancestor */
static inline int compareNodes(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODESELDATA*     nodeseldata,        /**< node selector data */
   SCIP_NODE*            node1,              /**< first node for comparison */
   SCIP_NODE*            node2               /**< second node for comparisons */
   )
{
   SCIP_Real score1;
   SCIP_Real score2;

   assert(node1 != node2);
   score1 = SCIPnodeGetScore(node1);
   score2 = SCIPnodeGetScore(node2);

   /* go back in the tree to find the two shallowest ancestors of node1 and node2 which share the same parent */

   while( SCIPnodeGetParent(node1) != SCIPnodeGetParent(node2) )
   {
      /* if the nodes have the same depth but not the same parent both pointers can be updated, otherwise only the deeper
       * node pointer is moved
       */
      if( SCIPnodeGetDepth(node1) == SCIPnodeGetDepth(node2) )
      {
         node1 = SCIPnodeGetParent(node1);
         node2 = SCIPnodeGetParent(node2);
      }
      else if( SCIPnodeGetDepth(node1) > SCIPnodeGetDepth(node2) )
         node1 = SCIPnodeGetParent(node1);
      else if( SCIPnodeGetDepth(node1) < SCIPnodeGetDepth(node2) )
         node2 = SCIPnodeGetParent(node2);

      assert(node1 != NULL);
      assert(node2 != NULL);
   }

   /* get UCT scores for both nodes */
   score1 = score1 + nodeGetUctScore(scip, node1, nodeseldata);
   score2 = score2 + nodeGetUctScore(scip, node2, nodeseldata);

   if( SCIPisEQ(scip, score1, score2) )
   {
      return 0;
   }
   else if( SCIPisGT(scip, score1, score2) )
      return -1;
   else
   {
      assert(SCIPisLT(scip, score1, score2));
      return 1;
   }
}

/** selects the best node among @p nodes with respect to UCT score */
static inline void selectBestNode(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE**           selnode,            /**< pointer to store the selected node, needs not be empty */
   SCIP_NODESELDATA*     nodeseldata,        /**< node selector data */
   SCIP_NODE**           nodes,              /**< array of nodes to select from */
   int                   nnodes              /**< size of the nodes array */
   )
{
   int n;

   assert(nnodes == 0 || nodes != NULL);
   assert(nnodes >= 0);
   assert(selnode != NULL);

   if( nnodes == 0 )
      return;

   /* loop over nodes, always keeping reference to the best found node so far */
   for( n = 0; n < nnodes; ++n )
   {
      assert(nodes[n] != NULL);
      /* update the selected node if the current node has a higher score */
      if( *selnode == NULL || compareNodes(scip, nodeseldata, *selnode, nodes[n]) < 0 )
         *selnode = nodes[n];
   }
}

/** keeps visits array large enough to save visits for all nodes in the branch and bound tree */
static inline SCIP_RETCODE ensureMemorySize(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODESELDATA*     nodeseldata         /**< node selector data */
   )
{
   assert(nodeseldata != NULL);

   /* if array has not been allocated yet, do this now with default initial capacity */
   if( nodeseldata->nodevisits == NULL )
   {
      SCIP_CALL( SCIPallocClearMemoryArray(scip, &nodeseldata->nodevisits, INITIALSIZE) ); /*lint !e506*/
      nodeseldata->sizenodevisits = INITIALSIZE;
   }

   /* if user node limit has not been reached yet, resize the visits array if necessary */
   if( nodeseldata->sizenodevisits < 2 * nodeseldata->nodelimit && nodeseldata->sizenodevisits < (int)(2 * SCIPgetNNodes(scip)))
   {
      int newcapacity;
      newcapacity = MIN(2 * nodeseldata->sizenodevisits, 2 * nodeseldata->nodelimit);

      SCIPdebugMsg(scip, "Resizing node visits array, old capacity: %d new capacity : %d\n", nodeseldata->sizenodevisits, newcapacity);
      assert(newcapacity > nodeseldata->sizenodevisits);

      SCIP_CALL( SCIPreallocMemoryArray(scip, &nodeseldata->nodevisits, newcapacity) );
      BMSclearMemoryArray(&(nodeseldata->nodevisits[nodeseldata->sizenodevisits]), newcapacity - nodeseldata->sizenodevisits); /*lint !e866*/

      nodeseldata->sizenodevisits = newcapacity;
   }

   return SCIP_OKAY;
}

#endif //UCT_H