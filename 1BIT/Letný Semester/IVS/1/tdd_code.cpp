//======= Copyright (c) 2024, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Test Driven Development - graph
//
// $NoKeywords: $ivs_project_1 $tdd_code.cpp
// $Author:     HUGO BOHÁCSEK <xbohach00@stud.fit.vutbr.cz>
// $Date:       $2024-02-14
//============================================================================//
/**
 * @file tdd_code.cpp
 * @author Martin Dočekal
 * @author Karel Ondřej
 * @author Hugo Bohácsek
 * @brief Implementace metod tridy reprezentujici graf.
 */

#include "tdd_code.h"


Graph::Graph(){}

Graph::~Graph(){
	clear();
}

std::vector<Node*> Graph::nodes() {
    return nodes_moje;
}

std::vector<Edge> Graph::edges() const{
    std::vector<Edge> edg;
	for (int i=0; i<edges_moje.size(); i++){
		edg.push_back(*edges_moje[i]);
	}
	return edg;
}

Node* Graph::addNode(size_t nodeId) {
	if (getNode(nodeId) != nullptr && !nodes_moje.empty()) {
		return nullptr;
	}
	Node* newNode = (Node*) malloc(sizeof (Node));
	newNode->id = nodeId;
	newNode->deg=0;
	newNode->color=0;
	//static Node newNode;
	//newNode.id = nodeId;
	
	nodes_moje.push_back(newNode);
	return newNode;
}

bool Graph::addEdge(const Edge& edge){
	if (edge.a == edge.b){return false;}
	if (Graph::containsEdge(edge)){
		return false;
	}
	if (getNode(edge.a) == nullptr){
		addNode(edge.a);
	}
	if (getNode(edge.b) == nullptr){
		addNode(edge.b);
	}
	
	Edge* edg = (Edge*)malloc(sizeof(Edge));
	edg->a = edge.a;
	edg->b = edge.b;
	
	
	
	getNode(edge.a)->deg++;
	getNode(edge.b)->deg++;
	
    edges_moje.push_back(edg);
	return true;
}

void Graph::addMultipleEdges(const std::vector<Edge>& edges) {
	for (int i = 0; i < edges.size(); i++){
		Graph::addEdge(edges[i]);
		if (!getNode(edges[i].a)){
			addNode(edges[i].a);
		}
		if (!getNode(edges[i].b)){
			addNode(edges[i].b);
		}
	}
}

Node* Graph::getNode(size_t nodeId){
	for (int i=0; i < nodes_moje.size(); i++){
		if (nodes_moje[i]->id == nodeId){
			return nodes_moje[i];
		}
	}
    return nullptr;
}

bool Graph::containsEdge(const Edge& edge) const{
	for (int i = 0; i < edges_moje.size(); i++){
		if (*edges_moje[i] == edge){
			return true;
		}
	}
    return false;
}

void Graph::removeNode(size_t nodeId){
	int index = -1;
	for (int i = 0; i < nodes_moje.size(); i++){
		if (nodes_moje[i]->id == nodeId){
			index = i;
			break;
		}
	}
	if (index == -1){
		throw std::out_of_range("Not found!");
	}
	int last_size = -1;
	while (last_size != edges_moje.size()){
		last_size = edges_moje.size();
		for (int i = 0; i < edges_moje.size(); i++){
			if (edges_moje[i]->a == nodeId || edges_moje[i]->b == nodeId){
				removeEdge(*edges_moje[i]);
			}
		}
	}/*
	for (int i = 0; i < edges_moje.size(); i++){
		if (edges_moje[i]->a == nodeId || edges_moje[i]->b == nodeId){
			removeEdge(*edges_moje[i]);
		}
	}
	for (int i = 0; i < edges_moje.size(); i++){
		if (edges_moje[i]->a == nodeId || edges_moje[i]->b == nodeId){
			removeEdge(*edges_moje[i]);
		}
	}
	for (int i = 0; i < edges_moje.size(); i++){
		if (edges_moje[i]->a == nodeId || edges_moje[i]->b == nodeId){
			removeEdge(*edges_moje[i]);
		}
	}
	for (int i = 0; i < edges_moje.size(); i++){
		if (edges_moje[i]->a == nodeId || edges_moje[i]->b == nodeId){
			removeEdge(*edges_moje[i]);
		}
	}*/
	
	free(nodes_moje[index]);
	nodes_moje.erase(nodes_moje.begin() + index);
}

void Graph::removeEdge(const Edge& edge){
	int index = -1;
	for (int i = 0; i < edges_moje.size(); i++){
		if (*edges_moje[i] == edge){
			index = i;
			break;
		}
	}
	if (index == -1){
		throw std::out_of_range("Not found!");
	}
	free(edges_moje[index]);
	edges_moje.erase(edges_moje.begin() + index);
}

size_t Graph::nodeCount() const{
	return const_cast<Graph*>(this)->nodes_moje.size();
}

size_t Graph::edgeCount() const{
    return edges_moje.size();
}

size_t Graph::nodeDegree(size_t nodeId) const{
	if (const_cast<Graph*>(this)->getNode(nodeId) == nullptr){throw std::out_of_range(":(");}
    return const_cast<Graph*>(this)->getNode(nodeId)->deg;
}

size_t Graph::graphDegree() const{
	size_t max = 0;
    for (int i=0; i < nodes_moje.size(); i++){
		if (nodes_moje[i]->deg > max) {
			max = nodes_moje[i]->deg;
		}
	}
	return max;
}

void Graph::coloring(){
	std::vector<int> farbicky = {};
	int carbicky = 1;
	int so_sebou_mam_stale = 0;
	for (int i=0; i < nodes_moje.size(); i++){
		carbicky = 1;
		farbicky.clear();
		for (int j = 0; j < edges_moje.size(); j++){
			if (edges_moje[j]->a == nodes_moje[i]->id){
				farbicky.push_back(getNode(edges_moje[j]->b)->color);
			}
			if(edges_moje[j]->b == nodes_moje[i]->id){
				farbicky.push_back(getNode(edges_moje[j]->a)->color);
			}
		}
		while (true){
			so_sebou_mam_stale = 0;
			for (int j = 0; j < farbicky.size(); j++){
				if (carbicky == farbicky[j]){
					so_sebou_mam_stale = 1;
					carbicky++;
				}
			}
			if (so_sebou_mam_stale == 0){
				break;
			}
		}
		nodes_moje[i]->color = carbicky;
	}
}

void Graph::clear() {
	for (int i = 0; i < nodes_moje.size(); i++){
		removeNode(nodes_moje[i]->id);
	}
	for (int i = 0; i < edges_moje.size(); i++){
		removeEdge(*edges_moje[i]);
	}
	for (int i = 0; i < edges_moje.size(); i++){
		free(edges_moje[i]);
	}
	for (int i = 0; i < nodes_moje.size(); i++){
		free(nodes_moje[i]);
	}
	edges_moje={};
	nodes_moje={};
}

/*** Konec souboru tdd_code.cpp ***/
