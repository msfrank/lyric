
#include <boost/graph/dominator_tree.hpp>

#include <lyric_optimizer/build_ssa.h>
#include <lyric_optimizer/internal/cfg_data.h>

typedef boost::property_map<lyric_optimizer::internal::Graph, boost::vertex_index_t>::type IndexMap;
typedef boost::iterator_property_map<std::vector<lyric_optimizer::internal::Vertex>::iterator, IndexMap> PredMap;

tempo_utils::Status
lyric_optimizer::build_ssa(const ControlFlowGraph &cfg)
{
    // auto priv = cfg.getPriv();
    // auto &graph = priv->graph;
    //
    // IndexMap indexMap(get(boost::vertex_index, graph));
    //
    // internal::VertexIterator it, end;
    // boost::tie(it, end) = boost::vertices(graph);
    // for (int j = 0; it != end; ++it, ++j) {
    //     internal::Vertex v = *it;
    //     boost::put(indexMap, v, j);
    // }
    //
    // std::vector<internal::Vertex> domTreePredVector(
    //     boost::num_vertices(graph),
    //     boost::graph_traits<internal::Graph>::null_vertex());
    // PredMap domTreePredMap = make_iterator_property_map(domTreePredVector.begin(), indexMap);
    //
    // boost::lengauer_tarjan_dominator_tree(graph, priv->entry->blockVertex, domTreePredMap);
    //
    // boost::tie(it, end) = boost::vertices(graph);
    // for (int j = 0; it != end; ++it, ++j) {
    //     internal::Vertex v = *it;
    //     auto d = domTreePredMap[v];
    //     if (d) {
    //         TU_LOG_INFO << "vertex " << (int) indexMap[v] << " dominated by " << (int) indexMap[domTreePredMap[v]];
    //     } else {
    //         TU_LOG_INFO << "vertex " << (int) indexMap[v] << " has no dominator";
    //     }
    // }

    return {};
}