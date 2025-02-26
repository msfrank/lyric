#ifndef LYRIC_OPTIMIZER_INTERNAL_CFG_TYPES_H
#define LYRIC_OPTIMIZER_INTERNAL_CFG_TYPES_H

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <tempo_utils/integer_types.h>

namespace lyric_optimizer::internal {

    enum class EdgeType {
        Invalid,
        Next,
        Branch,
        Jump,
        Return,
    };

    struct instruction_offset_t {
        typedef boost::vertex_property_tag kind;
    };

    struct target_id_t {
        typedef boost::edge_property_tag kind;
    };

    struct edge_type_t {
        typedef boost::edge_property_tag kind;
    };

    struct label_name_t {
        typedef boost::edge_property_tag kind;
    };

    // collection of vertex properties
    typedef boost::property<instruction_offset_t, tu_uint32,
            boost::property<boost::vertex_index_t, std::size_t
    >> VertexProperties;

    // collection of edge properties
    typedef boost::property<target_id_t, tu_uint32,
            boost::property<edge_type_t, EdgeType,
            boost::property<label_name_t, std::string
    >>> EdgeProperties;

    // concrete graph type
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VertexProperties, EdgeProperties> Graph;

    // vertex type for Graph
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

    // edge type for Graph
    typedef boost::graph_traits<Graph>::edge_descriptor Edge;

    // iterator types for Graph
    typedef boost::graph_traits<Graph>::vertex_iterator VertexIterator;
    typedef boost::graph_traits<Graph>::edge_iterator EdgeIterator;
    typedef boost::graph_traits<Graph>::in_edge_iterator InEdgeIterator;
    typedef boost::graph_traits<Graph>::out_edge_iterator OutEdgeIterator;

}

#endif // LYRIC_OPTIMIZER_INTERNAL_CFG_TYPES_H
