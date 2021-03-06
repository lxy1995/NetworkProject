#include "pearl_network.h"

#include "network.h"
#include "parameter.h"
#include "undirected_graph.h"
#include "neighbor_info.h"
#include "util.h"
#include "util_boost.h"

PearlNetwork::PearlNetwork(const Parameter& para) : Network(para) {
	std::vector<std::vector<std::string>> filenames = UtilBoost::parsePtree2layerAI2(
			UtilBoost::jsonFile2Ptree(para.get_net_injson()), para.get_part_str_length());
	for (int i = 0; i < filenames.size(); ++i) {
		std::vector<UGraphPtr> gs;
		for (int j = 0; j < filenames[i].size(); ++j) {
			gs.push_back(std::make_shared<UndirectedGraph>(
						para.get_net_inroot() + "/" + filenames[i][j], Util::vec2unset(_nodes)));
		}
		_ugraphss.push_back(gs);
	}
	_days = _ugraphss.size();
	if (_days <= 0) {
		std::cerr << "Error with _days = " << _days << "." << std::endl;
		exit(-1);
	} else {
		_parts = _ugraphss[0].size();
		for (int i = 1; i < _ugraphss.size(); ++i) {
			if (_ugraphss[i].size() != _parts) {
				std::cerr << "Error with _parts = " << _parts << " but _ugraphss[" << i << "].size() = " << _ugraphss[i].size() << "." << std::endl;
				exit(-1);
			}
		}
	}
	if (_nodes.size() == 0) {
		NodeSet all_nodes;
		for (int i = 0; i < _ugraphss.size(); ++i) {
			for (int j = 0; j < _ugraphss[i].size(); ++j) {
				all_nodes = Util::getUnion(all_nodes, _ugraphss[i][j]->get_node_names());
			}
		}
		_nodes = Util::unset2vec(all_nodes);
		update_node_map();
		for (int i = 0; i < _ugraphss.size(); ++i) {
			for (int j = 0; j < _ugraphss[i].size(); ++j) {
				_ugraphss[i][j]->add_nodes(Util::getDiff(all_nodes, _ugraphss[i][j]->get_node_names()));
			}
		}
	}
}

PearlNetwork::~PearlNetwork() {
}

NeighborList PearlNetwork::get_neighbor_list(const std::string& nodename, const int& day_index, const int& part_index) const {
	if (check_day_index(day_index) && check_part_index(part_index)) {
		return _ugraphss[day_index][part_index]->get_neighbor_list(nodename);
	} else {
		std::cerr << "Error: day_index = " << day_index << " and part_index = " << part_index << "." << std::endl;
		exit(-1);
	}
}

std::shared_ptr<UndirectedGraph> PearlNetwork::get_merged_graph() {
	if (!_merged_graph) {
		std::vector<std::shared_ptr<UndirectedGraph>> temp(_days * _parts, NULL);
		for (int i = 0; i < _ugraphss.size(); ++i) {
			for (int j = 0; j < _ugraphss[i].size(); ++j) {
				temp[i * _parts + j] = _ugraphss[i][j];
			}
		}
		_merged_graph = UndirectedGraph::merge(temp, true); // with_weight = true
	}
	return _merged_graph;
}
	
std::shared_ptr<UndirectedGraph> PearlNetwork::get_undirected_graph(const int& day_index, const int& part_index) const {
	if (check_day_index(day_index) && check_part_index(part_index)) {
		return _ugraphss[day_index][part_index];
	} else {
		return nullptr;
	}
}


