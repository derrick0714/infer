#include "all_simple_paths_visitor.hpp"

#include <iostream>
#include <vector>
#include <list>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/property_map/property_map.hpp>

#include <boost/graph/graph_utility.hpp>

using namespace std;
using namespace boost;

/*
typedef
	subgraph
		<adjacency_list
			<vecS,
			 vecS,
			 undirectedS,
			 no_property,
			 property<edge_index_t, int>
			>
		> UndirectedGraph;
*/
typedef
	adjacency_list
		<vecS,
		 vecS,
		 undirectedS,
		 no_property,
		 property<edge_index_t, int>
		> UndirectedGraph;
typedef graph_traits<UndirectedGraph>::vertex_descriptor Vertex;

int main()
{
	UndirectedGraph g;
	boost::add_edge(0, 1, g);
	boost::add_edge(1, 2, g);
	boost::add_edge(2, 3, g);
	boost::add_edge(3, 4, g);
	boost::add_edge(0, 5, g);
	boost::add_edge(5, 6, g);
	boost::add_edge(6, 7, g);
	boost::add_edge(5, 8, g);
	boost::add_edge(0, 9, g);
	boost::add_edge(9, 8, g);
	boost::add_edge(9, 10, g);
	boost::add_edge(0, 11, g);
	boost::add_edge(11, 10, g);
	boost::add_edge(0, 13, g);
	boost::add_edge(13, 12, g);
	boost::add_edge(0, 17, g);
	boost::add_edge(17, 16, g);
	boost::add_edge(16, 15, g);
	boost::add_edge(15, 14, g);
	boost::add_edge(17, 18, g);
	boost::add_edge(18, 19, g);
	boost::add_edge(0, 19, g);
	boost::add_edge(19, 20, g);
	boost::add_edge(21, 22, g);

	cerr << endl;
	Vertex search_root(0);
	vector<Vertex> related_hosts;
	related_hosts.push_back(Vertex(18));
	//related_hosts.push_back(Vertex(14));
	//related_hosts.push_back(Vertex(16));
	//related_hosts.push_back(Vertex(18));
	//related_hosts.push_back(Vertex(20));

	// DFSVisitor
	/*
	all_simple_paths_visitor<UndirectedGraph>
		vis(g,
			related_hosts.begin(),
			related_hosts.end());

	cerr << "depth_first_search():" << endl;

	depth_first_search(g,
					   visitor(vis).
					   root_vertex(search_root).
					   color_map(vis.color_map()));
	cerr << endl;

	UndirectedGraph &sub(vis.create_subgraph());
	print_graph(sub, get(vertex_index, sub));
	*/

	all_simple_paths_visitor<UndirectedGraph>
		vis2(g,
			related_hosts.begin(),
			related_hosts.end());
	cerr << "depth_first_visit():" << endl;
	depth_first_visit(g, search_root, vis2, vis2.color_map());
	cerr << endl;

	UndirectedGraph sub2(vis2.create_subgraph());
	print_graph(sub2, get(vertex_index, sub2));
	cout << endl;

	graph_traits<UndirectedGraph>::vertex_iterator vi, vi_end;
	graph_traits<UndirectedGraph>::out_edge_iterator out, out_end;
	for (tie(vi, vi_end) = vertices(sub2); vi != vi_end; ++vi) {
		cout << vis2.local_to_global(*vi) << " <--> ";
		for (tie(out, out_end) = out_edges(*vi, sub2); out != out_end; ++out) {
			cout << vis2.local_to_global(target(*out, sub2)) << " ";
		}
		cout << endl;
	}

	return 0;
}
