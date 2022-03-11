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
public:
	vector<Layer> layers;

	int M;
	int MMax;
	int MMax0;
	int eFConstructions;
	float mL;

	vector<Node*> allNodes;

	Hnsw()
	{
		M = 16;
		MMax = 16;
		eFConstructions = 200;

		MMax0 = MMax * 2;
		mL = 1 / log(0.8 * M);
	}

	Hnsw(int M, int MMax, int eFC)
	{
		this->M = M;
		this->MMax = MMax;
		this->eFConstructions = eFC;

		this->MMax0 = MMax * 2;
		this->mL = 1 / log(0.8 * M);
	}

	~Hnsw()
	{
		/*for (auto n : allNodes)
		{
			delete n;
		}*/
		//allNodes.clear();
	}

	/////////////////////////////// INSERT NODE ///////////////////////////////

	void Insert(Node* newNode)
	{
		allNodes.push_back(newNode);

		uint newNodeIndex = allNodes.size() - 1;
		int L = layers.size() - 1;
		int l = -log(((float)(rand() % 10000 + 1)) / 10000) * mL; //(0 - 9) * mL => 0 - 3

		if (newNodeIndex == 0)
		{

			while (l > L)
			{
				Layer newLayer = Layer(0);
				layers.push_back(newLayer);

				l--;
			}

			return;
		}

		NodeDist entryPoint = NodeDist(layers[L].entryPoint);
		entryPoint.SetDistance(allNodes[entryPoint.ID]->values, newNode->values);

		for (int lC = L; lC > l; lC--)
		{
			SearchLayerOne(newNode, entryPoint, lC);
		}

		vector<NodeDist> W;
		W.push_back(entryPoint);

		for (int lC = min(L, l); lC >= 0; lC--)
		{
			W = SearchLayer(newNode, W, eFConstructions, lC);

			vector<NodeDist> neighbors = SelectNeighborsHeuristic(newNodeIndex, W, M, false);

			for (auto& nbr : neighbors)
			{
				allNodes[newNodeIndex]->Insert(nbr.ID, lC);	//add bidirectionall connectionts from neighbors to q at layer lc
				allNodes[nbr.ID]->Insert(newNodeIndex, lC);

				Neighbours* nbrNbrs = allNodes[nbr.ID]->GetNeighboursAtLayer(lC);
				int eConn = nbrNbrs->Size();

				int MMax_ = MMax;

				if (lC == 0)
					MMax_ = MMax0;

				if (eConn > MMax_)
				{
					Neighbours* eNewCon = new Neighbours(nbrNbrs->layerID);

					vector<NodeDist> nbrNodes;
					for (auto& n : nbrNbrs->neighbours)
					{
						nbrNodes.push_back(NodeDist(n));
					}

					vector<uint> newNbs;
					for (auto& n : SelectNeighborsHeuristic(nbr.ID, nbrNodes, MMax_, true))
					{
						newNbs.push_back(n.ID);
					}

					eNewCon->neighbours = newNbs; // SelectNeighborsHeuristic(nbr, nbrNbrs->neighbours, MMax_);

					allNodes[nbr.ID]->SetNeighbours(eNewCon);
				}
			}

			//W = neighbors;
		}

		while (l > L)
		{
			Layer newLayer = Layer(newNodeIndex);
			layers.push_back(newLayer);

			l--;
		}

		if (allNodes.size() % 10000 == 0)
			printf("Inserted %d\n", allNodes.size());
	}

	void SearchLayerOne(Node* queryNode, NodeDist& entryPoint, int lC)
	{
		bool change = true;
		linearHash visitedNodes = linearHash();
		visitedNodes.clear();
		visitedNodes.insert(entryPoint.ID);

		while (change)
		{
			change = false;
			vector<uint> nbs = allNodes[entryPoint.ID]->GetNeighboursVectorAtLayer(lC);

			for (auto n : nbs)
			{
				if (!visitedNodes.get(n))
				{
					visitedNodes.insert(n);
					NodeDist newNode = NodeDist(n);
					newNode.SetDistance(allNodes[n]->values, queryNode->values);

					if (newNode.distance < entryPoint.distance)
					{
						change = true;
						entryPoint = newNode;
					}
				}
			}
		}
	}

	vector<NodeDist> SearchLayer(Node* queryNode, vector<NodeDist> entryPoints, int K, int layerC)
	{
		vector<NodeDist> candidateNodes = entryPoints;
		vector<NodeDist> nearestNodes = entryPoints;
		linearHash visitedNodes = linearHash();
		visitedNodes.clear();

		float nNDist;

		for (auto ep : entryPoints)
		{
			visitedNodes.insert(ep.ID);
		}

		std::make_heap(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSortNearest());
		std::make_heap(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSortFurthest());

		nNDist = nearestNodes.front().distance;
		while (!candidateNodes.empty())
		{
			auto c = candidateNodes.front();

			std::pop_heap(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSortFurthest());
			candidateNodes.pop_back();

			if (c.distance > nNDist)
				break;

			for (auto n : allNodes[c.ID]->GetNeighboursVectorAtLayer(layerC))
			{
				if (!visitedNodes.get(n))
				{
					visitedNodes.insert(n);

					NodeDist newNode = NodeDist(n);
					newNode.SetDistance(allNodes[n]->values, queryNode->values);

					if (newNode.distance < nNDist || nearestNodes.size() < K)
					{
						candidateNodes.push_back(newNode);
						std::push_heap(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSortFurthest());
						nearestNodes.push_back(newNode);
						std::push_heap(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSortNearest());

						if (nearestNodes.size() > K)
						{
							std::pop_heap(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSortNearest());
							nearestNodes.pop_back();
						}

						nNDist = nearestNodes.front().distance;
					}

				}
			}
		}

		return nearestNodes;
	}

	/*vector<uint> SelectNeighborsSimple(uint queryNode, vector<uint> neighbours, int M)
	{
		if (neighbours.size() <= M)
			return neighbours;

		vector<uint> sortedNodes = neighbours;

		for (auto& n : neighbours)
		{
			allNodes[n]->SetDistance(allNodes[queryNode]);
		}

		sort(sortedNodes.begin(), sortedNodes.end(), NodeSortNearest(allNodes));
		sortedNodes.erase(sortedNodes.begin() + M, sortedNodes.end());

		return sortedNodes;
	}*/

	vector<NodeDist> SelectNeighborsHeuristic(uint queryNode, vector<NodeDist> W, int M, bool computeDist)
	{
		if (W.size() < M)
		{
			return W;
		}

		if (computeDist)
		{
			for (auto& n : W)
			{
				n.SetDistance(allNodes[n.ID]->values, allNodes[queryNode]->values);
			}
		}

		sort(W.begin(), W.end(), NodeDistanceSortNearest());

		vector<NodeDist> R;

		int s = 2;
		int i = 2;

		R.emplace_back(W[0]);
		R.emplace_back(W[1]);

		while (i < W.size() && s < M)
		{
			bool q_is_close = true;
			for (int j = 0; j < R.size(); j++)
			{
				float dist = allNodes[W[i].ID]->GetDistance(allNodes[R[j].ID]->values);

				if (dist < W[i].distance)
				{
					q_is_close = false;
					break;
				}

			}

			if (q_is_close)
			{
				R.emplace_back(W[i]);
				s++;
			}

			i++;
		}

		return R;

	}

	/////////////////////////////// SEARCH NODE ///////////////////////////////

	vector<NodeDist> SearchLayerKNN(Node* queryNode, NodeDist& entryPoint, uint K)
	{
		vector<NodeDist> candidateNodes;
		vector<NodeDist> nearestNodes;
		linearHash visitedNodes = linearHash();
		visitedNodes.clear();

		float nNDist = entryPoint.distance;

		candidateNodes.push_back(entryPoint);
		nearestNodes.push_back(entryPoint);
		visitedNodes.insert(entryPoint.ID);

		std::make_heap(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSortNearest());
		std::make_heap(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSortFurthest());

		while (!candidateNodes.empty())
		{
			auto c = candidateNodes.front();

			std::pop_heap(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSortFurthest());
			candidateNodes.pop_back();

			if (c.distance > nNDist)
				break;

			for (auto n : allNodes[c.ID]->GetNeighboursVectorAtLayer(0))
			{
				if (!visitedNodes.get(n))
				{
					visitedNodes.insert(n);

					NodeDist newNode = NodeDist(n);
					newNode.SetDistance(allNodes[n]->values, queryNode->values);

					if (newNode.distance < nNDist || nearestNodes.size() < K)
					{
						candidateNodes.push_back(newNode);
						std::push_heap(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSortFurthest());
						nearestNodes.push_back(newNode);
						std::push_heap(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSortNearest());

						if (nearestNodes.size() > K)
						{
							std::pop_heap(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSortNearest());
							nearestNodes.pop_back();
						}

						nNDist = nearestNodes.front().distance;
					}


				}
			}
		}

		sort(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSortNearest());
		return nearestNodes;
	}

	vector<uint> KNNSearchIndex(Node* queryNode, int K, int efC)
	{
		int L = layers.size() - 1;
		NodeDist entryPoint = NodeDist(layers[L].entryPoint);
		entryPoint.SetDistance(allNodes[entryPoint.ID]->values, queryNode->values);

		for (int lC = L; lC >= 1; lC--)
		{
			SearchLayerOne(queryNode, entryPoint, lC);
		}

		vector<uint> nearestNodes;
		vector<NodeDist> nearestNodesIndexRef = SearchLayerKNN(queryNode, entryPoint, efC);

		for (int i = 0; i < K; i++)
		{
			nearestNodes.push_back(nearestNodesIndexRef[i].ID);
		}

		return nearestNodes;
	}

	/////////////////////////////// SEARCH FILTER ///////////////////////////////

	void SearchLayerOneFilter(Node* queryNode, NodeDist& entryPoint, vector<DimFilter> filters, int lC)
	{
		bool change = true;
		linearHash visitedNodes = linearHash();
		visitedNodes.clear();
		visitedNodes.insert(entryPoint.ID);

		while (change)
		{
			change = false;
			vector<uint> nbs = allNodes[entryPoint.ID]->GetNeighboursVectorAtLayer(lC);

			for (auto n : nbs)
			{
				if (!visitedNodes.get(n))
				{
					visitedNodes.insert(n);

					NodeDist newNode = NodeDist(n);
					newNode.SetDistance(allNodes[n]->values, queryNode->values);

					if (newNode.distance < entryPoint.distance)
					{
						if (DimFilterHelper::IsVectorValid(filters, allNodes[n]->values))
						{
							change = true;
							entryPoint = newNode;
						}
					}
				}
			}
		}
	}

	vector<NodeDist> SearchLayerFilter(Node* queryNode, NodeDist& entryPoint, vector<DimFilter> filters, uint K, uint KNN)
	{
		vector<NodeDist> candidateNodes;
		vector<NodeDist> nearestNodes;
		linearHash visitedNodes = linearHash();
		visitedNodes.clear();

		float nNDist = entryPoint.distance;

		candidateNodes.push_back(entryPoint);
		nearestNodes.push_back(entryPoint);
		visitedNodes.insert(entryPoint.ID);

		std::make_heap(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSortNearest());
		std::make_heap(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSortFurthest());

		vector<NodeDist> filteredNodes;
		if (DimFilterHelper::IsVectorValid(filters, allNodes[entryPoint.ID]->values))
		{
			filteredNodes.push_back(entryPoint);
		}
		std::make_heap(filteredNodes.begin(), filteredNodes.end(), NodeDistanceSortNearest());

		while (!candidateNodes.empty())
		{
			auto c = candidateNodes.front();

			std::pop_heap(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSortFurthest());
			candidateNodes.pop_back();

			if (c.distance > nNDist && filteredNodes.size() == KNN)
				break;

			for (auto n : allNodes[c.ID]->GetNeighboursVectorAtLayer(0))
			{
				if (!visitedNodes.get(n))
				{
					visitedNodes.insert(n);

					NodeDist newNode = NodeDist(n);
					newNode.SetDistance(allNodes[n]->values, queryNode->values);

					if (DimFilterHelper::IsVectorValid(filters, allNodes[n]->values))
					{
						filteredNodes.push_back(newNode);
						std::push_heap(filteredNodes.begin(), filteredNodes.end(), NodeDistanceSortNearest());

						if (filteredNodes.size() > KNN)
						{
							std::pop_heap(filteredNodes.begin(), filteredNodes.end(), NodeDistanceSortNearest());
							filteredNodes.pop_back();
						}
					}

					if (newNode.distance < nNDist || nearestNodes.size() < K)
					{
						candidateNodes.push_back(newNode);
						std::push_heap(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSortFurthest());

						nearestNodes.push_back(newNode);
						std::push_heap(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSortNearest());

						if (nearestNodes.size() > K)
						{
							std::pop_heap(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSortNearest());
							nearestNodes.pop_back();
						}

						nNDist = nearestNodes.front().distance;
					}
				}
			}
		}

		sort(filteredNodes.begin(), filteredNodes.end(), NodeDistanceSortNearest());
		return filteredNodes;
	}

	vector<uint> KNNFilter(Node* queryNode, vector<DimFilter> filters, int K, int efC)
	{
		int L = layers.size() - 1;
		NodeDist entryPoint = NodeDist(layers[L].entryPoint);
		entryPoint.SetDistance(allNodes[entryPoint.ID]->values, queryNode->values);

		for (int lC = L; lC >= 1; lC--)
		{
			SearchLayerOne(queryNode, entryPoint, lC);
			//SearchLayerOneFilter(queryNode, entryPoint, filters, lC);
		}

		vector<uint> nearestNodes;
		vector<NodeDist> nearestNodesIndexRef = SearchLayerFilter(queryNode, entryPoint, filters, efC, K);

		//if (nearestNodesIndexRef.size() < K)
		//	cout << "Found only " << nearestNodesIndexRef.size() << "/" << K << " nodes" << endl;

		for (int i = 0; i < nearestNodesIndexRef.size(); i++)
		{
			nearestNodes.push_back(nearestNodesIndexRef[i].ID);
		}

		return nearestNodes;
	}

	/////////////////////////////// STATISTICS ///////////////////////////////

	void PrintInfo(int n)
	{
		if (n == 0)
			n = allNodes.size();

		for (int i = 0; i < n; i++)
		{
			printf("%d: ", i);

			for (auto& n : allNodes[i]->GetNeighboursVectorAtLayer(0))
			{
				printf("%d ", n);
			}

			printf("\n");
		}

		printf("\n");
	}

	void PrintInfoSorted(int n)
	{
		if (n == 0)
			n = allNodes.size();

		for (int i = 0; i < n; i++)
		{
			printf("%d: ", i);

			vector<uint> nbs = allNodes[i]->GetNeighboursVectorAtLayer(0);

			sort(nbs.begin(), nbs.end());

			for (auto& n : nbs)
			{
				printf("%d  ", n);
			}

			printf("\n");
		}

		printf("\n\n");
	}

	void SavePrint(int max_count, std::string name)
	{
		if (max_count == 0)
			max_count = allNodes.size();

		std::ofstream MyFile(name);

		for (int i = 0; i < max_count; i++) {
			if (i % 5000 == 0)
			{
				std::cout << i << " / " << max_count << std::endl;
			}

			std::string line = std::to_string(i) + ": ";

			vector<uint> nbs = allNodes[i]->GetNeighboursVectorAtLayer(0);

			sort(nbs.begin(), nbs.end());

			for (auto& n : nbs)
			{
				line += std::to_string(n) + "  ";
			}

			line += "\n";

			MyFile << line;
		}

		MyFile.close();
	}

};