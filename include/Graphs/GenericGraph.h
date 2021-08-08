//===- CenericGraph.h -- Generic graph ---------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013-2017>  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//

/*
 * GenericGraph.h
 *
 *  Created on: Mar 19, 2014
 *      Author: Yulei Sui
 */

#ifndef GENERICGRAPH_H_
#define GENERICGRAPH_H_

#include "Util/BasicTypes.h"

namespace SVF
{

/*!
 * Generic edge on the graph as base class
 */
template<class NodeTy>
class GenericEdge
{

public:
    /// Node type
    typedef NodeTy NodeType;
    /// Edge Flag
    /// Edge format as follows (from lowest bit):
    ///	(1) 0-7 bits encode an edge kind (allow maximum 16 kinds)
    /// (2) 8-63 bits encode a callsite instruction
    typedef u64_t GEdgeFlag;
    typedef s32_t GEdgeKind;
private:
    NodeTy* src;		///< source node
    NodeTy* dst;		///< destination node
    GEdgeFlag edgeFlag;	///< edge kind

public:
    /// Constructor
    GenericEdge(NodeTy* s, NodeTy* d, GEdgeFlag k): src(s),dst(d), edgeFlag(k)
    {

    }

    /// Destructor
    virtual ~GenericEdge()
    {
    }

    ///  get methods of the components
    //@{
    inline NodeID getSrcID() const
    {
        return src->getId();
    }
    inline NodeID getDstID() const
    {
        return dst->getId();
    }
    inline GEdgeKind getEdgeKind() const
    {
        return (EdgeKindMask & edgeFlag);
    }
    NodeType* getSrcNode() const
    {
        return src;
    }
    NodeType* getDstNode() const
    {
        return dst;
    }
    //@}

    /// Add the hash function for std::set (we also can overload operator< to implement this)
    //  and duplicated elements in the set are not inserted (binary tree comparison)
    //@{
    typedef struct equalGEdge
    {
        bool operator()(const GenericEdge<NodeType>* lhs, const GenericEdge<NodeType>* rhs) const
        {
            if (lhs->edgeFlag != rhs->edgeFlag)
                return lhs->edgeFlag < rhs->edgeFlag;
            else if (lhs->getSrcID() != rhs->getSrcID())
                return lhs->getSrcID() < rhs->getSrcID();
            else
                return lhs->getDstID() < rhs->getDstID();
        }
    } equalGEdge;

    inline bool operator==(const GenericEdge<NodeType>* rhs) const
    {
        return (rhs->edgeFlag == this->edgeFlag && rhs->getSrcID() == this->getSrcID()
                && rhs->getDstID() == this->getDstID());
    }
    //@}

protected:
    static constexpr unsigned char EdgeKindMaskBits = 8;  ///< We use the lower 8 bits to denote edge kind
    static constexpr u64_t EdgeKindMask = (~0ULL) >> (64 - EdgeKindMaskBits);
};


/*!
 * Generic node on the graph as base class
 */
template<class NodeTy,class EdgeTy>
class GenericNode
{

public:
    typedef NodeTy NodeType;
    typedef EdgeTy EdgeType;
    /// Edge kind
    typedef s32_t GNodeK;
    typedef OrderedSet<EdgeType*, typename EdgeType::equalGEdge> GEdgeSetTy;
    /// Edge iterator
    ///@{
    typedef typename GEdgeSetTy::iterator iterator;
    typedef typename GEdgeSetTy::const_iterator const_iterator;
    ///@}

private:
    NodeID id;		///< Node ID
    GNodeK nodeKind;	///< Node kind

    GEdgeSetTy InEdges; ///< all incoming edge of this node
    GEdgeSetTy OutEdges; ///< all outgoing edge of this node

public:
    /// Constructor
    GenericNode(NodeID i, GNodeK k): id(i),nodeKind(k)
    {

    }

    /// Destructor
    virtual ~GenericNode()
    {

    }

    /// Get ID
    inline NodeID getId() const
    {
        return id;
    }

    /// Get node kind
    inline GNodeK getNodeKind() const
    {
        return nodeKind;
    }

    /// Get incoming/outgoing edge set
    ///@{
    inline const GEdgeSetTy& getOutEdges() const
    {
        return OutEdges;
    }
    inline const GEdgeSetTy& getInEdges() const
    {
        return InEdges;
    }
    ///@}

    /// Has incoming/outgoing edge set
    //@{
    inline bool hasIncomingEdge() const
    {
        return (InEdges.empty() == false);
    }
    inline bool hasOutgoingEdge() const
    {
        return (OutEdges.empty() == false);
    }
    //@}

    ///  iterators
    //@{
    inline iterator OutEdgeBegin()
    {
        return OutEdges.begin();
    }
    inline iterator OutEdgeEnd()
    {
        return OutEdges.end();
    }
    inline iterator InEdgeBegin()
    {
        return InEdges.begin();
    }
    inline iterator InEdgeEnd()
    {
        return InEdges.end();
    }
    inline const_iterator OutEdgeBegin() const
    {
        return OutEdges.begin();
    }
    inline const_iterator OutEdgeEnd() const
    {
        return OutEdges.end();
    }
    inline const_iterator InEdgeBegin() const
    {
        return InEdges.begin();
    }
    inline const_iterator InEdgeEnd() const
    {
        return InEdges.end();
    }
    //@}

    /// Iterators used for SCC detection, overwrite it in child class if necessory
    //@{
    virtual inline iterator directOutEdgeBegin()
    {
        return OutEdges.begin();
    }
    virtual inline iterator directOutEdgeEnd()
    {
        return OutEdges.end();
    }
    virtual inline iterator directInEdgeBegin()
    {
        return InEdges.begin();
    }
    virtual inline iterator directInEdgeEnd()
    {
        return InEdges.end();
    }

    virtual inline const_iterator directOutEdgeBegin() const
    {
        return OutEdges.begin();
    }
    virtual inline const_iterator directOutEdgeEnd() const
    {
        return OutEdges.end();
    }
    virtual inline const_iterator directInEdgeBegin() const
    {
        return InEdges.begin();
    }
    virtual inline const_iterator directInEdgeEnd() const
    {
        return InEdges.end();
    }
    //@}

    /// Add incoming and outgoing edges
    //@{
    inline bool addIncomingEdge(EdgeType* inEdge)
    {
        return InEdges.insert(inEdge).second;
    }
    inline bool addOutgoingEdge(EdgeType* outEdge)
    {
        return OutEdges.insert(outEdge).second;
    }
    //@}

    /// Remove incoming and outgoing edges
    ///@{
    inline Size_t removeIncomingEdge(EdgeType* edge)
    {
        iterator it = InEdges.find(edge);
        assert(it != InEdges.end() && "can not find in edge in SVFG node");
        return InEdges.erase(edge);
    }
    inline Size_t removeOutgoingEdge(EdgeType* edge)
    {
        iterator it = OutEdges.find(edge);
        assert(it != OutEdges.end() && "can not find out edge in SVFG node");
        return OutEdges.erase(edge);
    }
    ///@}

    /// Find incoming and outgoing edges
    //@{
    inline EdgeType* hasIncomingEdge(EdgeType* edge) const
    {
        const_iterator it = InEdges.find(edge);
        if (it != InEdges.end())
            return *it;
        else
            return nullptr;
    }
    inline EdgeType* hasOutgoingEdge(EdgeType* edge) const
    {
        const_iterator it = OutEdges.find(edge);
        if (it != OutEdges.end())
            return *it;
        else
            return nullptr;
    }
    //@}
};

/*
 * Generic graph for program representation
 * It is base class and needs to be instantiated
 */
template<class NodeTy,class EdgeTy>
class GenericGraph
{

public:
    typedef NodeTy NodeType;
    typedef EdgeTy EdgeType;
    /// NodeID to GenericNode map
    typedef Map<NodeID, NodeType*> IDToNodeMapTy;

    /// Node Iterators
    //@{
    typedef typename IDToNodeMapTy::iterator iterator;
    typedef typename IDToNodeMapTy::const_iterator const_iterator;
    //@}

    /// Constructor
    GenericGraph(): edgeNum(0),nodeNum(0)
    {
    }

    /// Destructor
    virtual ~GenericGraph()
    {
        destroy();
    }

    /// Release memory
    void destroy()
    {
        for (iterator I = IDToNodeMap.begin(), E = IDToNodeMap.end(); I != E; ++I){
            // NodeType* node = I->second;
            // for(typename NodeType::iterator it = node->InEdgeBegin(), eit = node->InEdgeEnd(); it!=eit; ++it)
            //         delete *it;
        }
        for (iterator I = IDToNodeMap.begin(), E = IDToNodeMap.end(); I != E; ++I)
            delete I->second;
    }
    /// Iterators
    //@{
    inline iterator begin()
    {
        return IDToNodeMap.begin();
    }
    inline iterator end()
    {
        return IDToNodeMap.end();
    }
    inline const_iterator begin() const
    {
        return IDToNodeMap.begin();
    }
    inline const_iterator end() const
    {
        return IDToNodeMap.end();
    }
    //}@

    /// Add a Node
    inline void addGNode(NodeID id, NodeType* node)
    {
        IDToNodeMap[id] = node;
        nodeNum++;
    }

    /// Get a node
    inline NodeType* getGNode(NodeID id) const
    {
        const_iterator it = IDToNodeMap.find(id);
        assert(it != IDToNodeMap.end() && "Node not found!");
        return it->second;
    }

    /// Has a node
    inline bool hasGNode(NodeID id) const
    {
        const_iterator it = IDToNodeMap.find(id);
        return it != IDToNodeMap.end();
    }

    /// Delete a node
    inline void removeGNode(NodeType* node)
    {
        assert(node->hasIncomingEdge() == false
               && node->hasOutgoingEdge() == false
               && "node which have edges can't be deleted");
        iterator it = IDToNodeMap.find(node->getId());
        assert(it != IDToNodeMap.end() && "can not find the node");
        IDToNodeMap.erase(it);
    }

    /// Get total number of node/edge
    inline u32_t getTotalNodeNum() const
    {
        return nodeNum;
    }
    inline u32_t getTotalEdgeNum() const
    {
        return edgeNum;
    }
    /// Increase number of node/edge
    inline void incNodeNum()
    {
        nodeNum++;
    }
    inline void incEdgeNum()
    {
        edgeNum++;
    }

protected:
    IDToNodeMapTy IDToNodeMap; ///< node map

public:
    u32_t edgeNum;		///< total num of node
    u32_t nodeNum;		///< total num of edge
};

} // End namespace SVF

/* !
 * GraphTraits specializations for generic graph algorithms.
 * Provide graph traits for tranversing from a node using standard graph traversals.
 */
namespace llvm
{

/*!
 * GraphTraits for nodes
 */
template<class NodeTy,class EdgeTy> struct GraphTraits<SVF::GenericNode<NodeTy,EdgeTy>*  >
{
    typedef NodeTy NodeType;
    typedef EdgeTy EdgeType;

    static inline NodeType* edge_dest(const EdgeType* E)
    {
        return E->getDstNode();
    }

    // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
    typedef mapped_iterator<typename SVF::GenericNode<NodeTy,EdgeTy>::iterator, decltype(&edge_dest)> ChildIteratorType;

    static NodeType* getEntryNode(NodeType* pagN)
    {
        return pagN;
    }

    static inline ChildIteratorType child_begin(const NodeType* N)
    {
        return map_iterator(N->OutEdgeBegin(), &edge_dest);
    }
    static inline ChildIteratorType child_end(const NodeType* N)
    {
        return map_iterator(N->OutEdgeEnd(), &edge_dest);
    }
    static inline ChildIteratorType direct_child_begin(const NodeType *N)
    {
        return map_iterator(N->directOutEdgeBegin(), &edge_dest);
    }
    static inline ChildIteratorType direct_child_end(const NodeType *N)
    {
        return map_iterator(N->directOutEdgeEnd(), &edge_dest);
    }
};

/*!
 * Inverse GraphTraits for node which is used for inverse traversal.
 */
template<class NodeTy,class EdgeTy>
struct GraphTraits<Inverse<SVF::GenericNode<NodeTy,EdgeTy>* > >
{
    typedef NodeTy NodeType;
    typedef EdgeTy EdgeType;

    static inline NodeType* edge_dest(const EdgeType* E)
    {
        return E->getSrcNode();
    }

    // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
    typedef mapped_iterator<typename SVF::GenericNode<NodeTy,EdgeTy>::iterator, decltype(&edge_dest)> ChildIteratorType;

    static inline NodeType* getEntryNode(Inverse<NodeType* > G)
    {
        return G.Graph;
    }

    static inline ChildIteratorType child_begin(const NodeType* N)
    {
        return map_iterator(N->InEdgeBegin(), &edge_dest);
    }
    static inline ChildIteratorType child_end(const NodeType* N)
    {
        return map_iterator(N->InEdgeEnd(), &edge_dest);
    }

    static inline unsigned getNodeID(const NodeType* N)
    {
        return N->getId();
    }
};

/*!
 * GraphTraints
 */
template<class NodeTy,class EdgeTy> struct GraphTraits<SVF::GenericGraph<NodeTy,EdgeTy>* > : public GraphTraits<SVF::GenericNode<NodeTy,EdgeTy>*  >
{
    typedef SVF::GenericGraph<NodeTy,EdgeTy> GenericGraphTy;
    typedef NodeTy NodeType;
    typedef EdgeTy EdgeType;

    static NodeType* getEntryNode(GenericGraphTy* pag)
    {
        return nullptr; // return null here, maybe later we could create a dummy node
    }

    typedef std::pair<SVF::NodeID, NodeType*> PairTy;
    static inline NodeType* deref_val(PairTy P)
    {
        return P.second;
    }

    // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
    typedef mapped_iterator<typename GenericGraphTy::iterator, decltype(&deref_val)> nodes_iterator;

    static nodes_iterator nodes_begin(GenericGraphTy *G)
    {
        return map_iterator(G->begin(), &deref_val);
    }
    static nodes_iterator nodes_end(GenericGraphTy *G)
    {
        return map_iterator(G->end(), &deref_val);
    }

    static unsigned graphSize(GenericGraphTy* G)
    {
        return G->getTotalNodeNum();
    }

    static inline unsigned getNodeID(NodeType* N)
    {
        return N->getId();
    }
    static NodeType* getNode(GenericGraphTy *G, SVF::NodeID id)
    {
        return G->getGNode(id);
    }
};

} // End namespace llvm

#endif /* GENERICGRAPH_H_ */
