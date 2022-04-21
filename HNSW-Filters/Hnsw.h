#pragma once

#include "Node.h"
#include "Layer.h"
#include "DimFilter.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <tuple>
#include <chrono>

using namespace std;

class Hnsw
{
private:
	vector<Layer> layers;
	vector<Node*> allNodes;

	int M;
	int MMax;
	int MMax0;
	int eFConstructions;
	float mL;

	void SearchLayerOne(Node* queryNode, NodeDist& entryPoint, int lC);
	vector<NodeDist> SearchLayer(Node* queryNode, vector<NodeDist> entryPoints, int K, int layerC);
	vector<NodeDist> SelectNeighborsSimple(uint queryNode, vector<NodeDist> neighbours, int M);
	vector<NodeDist> SelectNeighborsHeuristic(uint queryNode, vector<NodeDist> W, int M, bool computeDist);

	vector<NodeDist> SearchLayerKNN(Node* queryNode, NodeDist& entryPoint, uint K);

	void SearchLayerOneFilter(Node* queryNode, NodeDist& entryPoint, vector<DimFilter> filters, int lC);
	vector<NodeDist> SearchLayerFilter(Node* queryNode, NodeDist& entryPoint, vector<DimFilter> filters, uint K, uint KNN);

public:

	Hnsw();
	Hnsw(int M, int MMax, int eFC);
	~Hnsw();

	void Insert(Node* newNode);
	
	vector<uint> KNN(Node* queryNode, int K, int efC);

	vector<uint> KNNFilter(Node* queryNode, vector<DimFilter> filters, int K, int efC);

	void PrintInfo(int n);
	void PrintInfoSorted(int n);
	void SavePrint(int max_count, string name);
};