#ifndef INFER_INCLUDE_ALL_SIMPLE_PATHS_VISITOR_HPP_
#define INFER_INCLUDE_ALL_SIMPLE_PATHS_VISITOR_HPP_

#include <vector>
#include <set>
#include <list>

#include <boost/graph/depth_first_search.hpp>
#include <boost/bimap.hpp>

template <typename Graph>
class all_simple_paths_visitor : public boost::default_dfs_visitor {
  public:
	typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_type;
	typedef typename boost::graph_traits<Graph>::edge_descriptor edge_type;
	typedef
		boost::iterator_property_map
			<
				std::vector<boost::default_color_type>::iterator,
				typename boost::property_map<Graph,	boost::vertex_index_t>::type
			> color_map_type;

	class terminator_function_t {
	  public:
		terminator_function_t(const all_simple_paths_visitor<Graph> *vis)
			:_vis(vis)
		{}

		bool operator() (vertex_type, const Graph &) const {
			if (_vis->cur_search_depth() < 3) {
				return false;
			}

			return true;
		}

	  private:
		const all_simple_paths_visitor<Graph> *_vis;
	};


	template <typename VertexIterator>
	all_simple_paths_visitor(Graph &g,
							 VertexIterator related_begin,
							 VertexIterator related_end)
		:_g(g),
		 _related_hosts(new std::set<vertex_type>(related_begin, related_end)),
		 _root_node(),
		 _search_path(new std::list<vertex_type>()),
		 _colors(new std::vector<boost::default_color_type>(
			num_vertices(g))),
		 _color_map(_colors->begin(), get(boost::vertex_index, g)),
		 _subgraph_edges(new subgraph_edges_type()),
		 _subgraph_vertex_map(new subgraph_vertex_map_type()),
		 _cur_internal(new bool(false))
	{}

	void start_vertex(vertex_type v, const Graph &) {
		_root_node = v;
		_search_path->clear();
		*_cur_internal = false;
	}

	void discover_vertex(vertex_type v, const Graph &) {
		_search_path->push_back(v);
		*_cur_internal = !(*_cur_internal);
		//cerr << "discover: " << v << endl;
	}

	void finish_vertex(vertex_type v, const Graph &) {
		//cerr << "finish:   " << v << endl;

		//std::cerr << "search path depth: " << _search_path->size() << std::endl;

		if (_related_hosts->find(v) != _related_hosts->end()) {
		/*
			std::cerr << "GOT ONE!" << std::endl;
			std::cerr << "\t";
			for (typename std::list<vertex_type>::const_iterator i(_search_path->begin());
				 i != _search_path->end();
				 ++i)
			{
				std::cerr << *i << " - ";
			}
			std::cerr << std::endl;
		*/

			//_subgraph_vertices->insert(_search_path->begin(), _search_path->end());
			for (typename std::list<vertex_type>::const_iterator
					j(_search_path->begin()),
					i(j++);
				 j != _search_path->end();
				 ++i, ++j)
			{
				if (_subgraph_vertex_map->right.find(*i) ==
						_subgraph_vertex_map->right.end())
				{
					_subgraph_vertex_map->insert(
						subgraph_vertex_map_type::value_type(
							_subgraph_vertex_map->right.size(), *i));
				}
				if (_subgraph_vertex_map->right.find(*j) ==
						_subgraph_vertex_map->right.end())
				{
					_subgraph_vertex_map->insert(
						subgraph_vertex_map_type::value_type(
							_subgraph_vertex_map->right.size(), *j));
				}

				if (*i < *j) {
					_subgraph_edges->insert(
						subgraph_edges_type::value_type(
							_subgraph_vertex_map->right.at(*i),
							_subgraph_vertex_map->right.at(*j)));
				}
				else {
					_subgraph_edges->insert(
						subgraph_edges_type::value_type(
							_subgraph_vertex_map->right.at(*j),
							_subgraph_vertex_map->right.at(*i)));
				}
			}
		}

		_search_path->pop_back();
		*_cur_internal = !(*_cur_internal);

		if (v != _root_node) {
			//mark it as white
			put(_color_map,
				v,
				boost::color_traits<boost::default_color_type>::white());
		}
	}

	color_map_type color_map() {
		return _color_map;
		//return color_map_type(_colors->begin(), get(boost::vertex_index, _g));
	}

	bool cur_internal() const {
		return *_cur_internal;
	}

	size_t cur_search_depth() const {
		return _search_path->size();
	}

	terminator_function_t terminator_function() {
		return terminator_function_t(this);
	}

	Graph create_subgraph() {
		//cerr << endl << "create_subgraph(): " << _subgraph_vertices->size() << " vertices..." << endl;
		/*
		for (typename std::set<vertex_type>::const_iterator i(_subgraph_vertices->begin());
			 i != _subgraph_vertices->end();
			 ++i)
		{
			std::cerr << *i << " " << std::endl;
		}
		*/
		return Graph(_subgraph_edges->begin(),
					 _subgraph_edges->end(),
					 _subgraph_vertex_map->size());
		//return _g.create_subgraph(_subgraph_vertices->begin(),
		//						  _subgraph_vertices->end());
	}

	vertex_type local_to_global(vertex_type v) const {
		return _subgraph_vertex_map->left.at(v);
	}

	vertex_type global_to_local(vertex_type v) const {
		return _subgraph_vertex_map->right.at(v);
	}

	bool is_valid_internal_host_node(vertex_type v) const {
		if (v == _root_node ||
				_related_hosts->find(v) != _related_hosts->end())
		{
			return true;
		}

		return false;
	}

	void reset() {
		// TODO FIXME other stuff needs to be done here
	}

  private:
	typedef std::set<std::pair<size_t, size_t> > subgraph_edges_type;
	typedef boost::bimap<size_t, size_t> subgraph_vertex_map_type;

	Graph &_g;
	std::set<vertex_type> *_related_hosts;

	vertex_type _root_node;
	std::list<vertex_type> *_search_path;
	std::vector<boost::default_color_type> *_colors;
	color_map_type _color_map;
	subgraph_edges_type *_subgraph_edges;
	subgraph_vertex_map_type *_subgraph_vertex_map;
	bool *_cur_internal;
};

#endif
