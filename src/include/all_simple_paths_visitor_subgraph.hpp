#ifndef INFER_INCLUDE_ALL_SIMPLE_PATHS_VISITOR_HPP_
#define INFER_INCLUDE_ALL_SIMPLE_PATHS_VISITOR_HPP_

#include <vector>
#include <set>
#include <list>

#include <boost/graph/depth_first_search.hpp>

template <typename Graph>
class all_simple_paths_visitor_subgraph : public boost::default_dfs_visitor {
  public:
	typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_type;
	typedef typename boost::graph_traits<Graph>::edge_descriptor edge_type;
	typedef
		boost::iterator_property_map
			<
				std::vector<boost::default_color_type>::iterator,
				typename boost::property_map<Graph,	boost::vertex_index_t>::type
			> color_map_type;


	template <typename VertexIterator>
	all_simple_paths_visitor_subgraph(Graph &g,
							 VertexIterator related_begin,
							 VertexIterator related_end)
		:_g(g),
		 _related_hosts(new std::set<vertex_type>(related_begin, related_end)),
		 _root_node(),
		 _search_path(new std::list<vertex_type>()),
		 _colors(new std::vector<boost::default_color_type>(
			num_vertices(g))),
		 _color_map(_colors->begin(), get(boost::vertex_index, g)),
		 _subgraph_vertices(new std::set<vertex_type>())
	{}

	void start_vertex(vertex_type v, const Graph &) {
		_root_node = v;
		_search_path->clear();
	}

	void discover_vertex(vertex_type v, const Graph &) {
		_search_path->push_back(v);
		//cerr << "discover: " << v << endl;
	}

	void finish_vertex(vertex_type v, const Graph &) {
		//cerr << "finish:   " << v << endl;

		if (_related_hosts->find(v) != _related_hosts->end()) {
			/*
			cerr << "\t";
			for (typename std::list<vertex_type>::const_iterator i(_search_path->begin());
				 i != _search_path->end();
				 ++i)
			{
				cerr << *i << " - ";
			}
			cerr << endl;
			*/
			_subgraph_vertices->insert(_search_path->begin(), _search_path->end());
		}

		_search_path->pop_back();

		//mark it as white
		put(_color_map,
			v,
			boost::color_traits<boost::default_color_type>::white());
	}

	color_map_type color_map() {
		return _color_map;
		//return color_map_type(_colors->begin(), get(boost::vertex_index, _g));
	}

	Graph & create_subgraph() {
		//cerr << endl << "create_subgraph(): " << _subgraph_vertices->size() << " vertices..." << endl;
		/*
		for (typename std::set<vertex_type>::const_iterator i(_subgraph_vertices->begin());
			 i != _subgraph_vertices->end();
			 ++i)
		{
			std::cerr << *i << " " << std::endl;
		}
		*/
		return _g.create_subgraph(_subgraph_vertices->begin(),
								  _subgraph_vertices->end());
	}

	void reset() {
		// TODO FIXME other stuff needs to be done here
	}

  private:
	Graph &_g;
	std::set<vertex_type> *_related_hosts;

	vertex_type _root_node;
	std::list<vertex_type> *_search_path;
	std::vector<boost::default_color_type> *_colors;
	color_map_type _color_map;

	std::set<vertex_type> *_subgraph_vertices;
};

#endif
