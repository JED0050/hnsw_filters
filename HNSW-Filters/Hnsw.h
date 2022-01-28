#pragma once
#include<vector>
#include"Node.h"
#include"Layer.h"
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

	void Insert(Node* newNode)
	{
		allNodes.push_back(newNode);

		unsigned int newNodeIndex = allNodes.size() - 1;
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

		unsigned int entryPoint = layers[L].entryPoint;

		for (int lC = L; lC > l; lC--)
		{
			entryPoint = SearchLayerOne(newNode, entryPoint, lC);
		}

		vector<unsigned int> W;
		W.push_back(entryPoint);

		for (int lC = min(L, l); lC >= 0; lC--)
		{
			W = SearchLayerR(newNode, W, eFConstructions, lC);

			vector<unsigned int> neighbors = SelectNeighborsHeuristic(newNodeIndex, W, M);

			for (auto& nbr : neighbors)
			{
				allNodes[newNodeIndex]->Insert(nbr, lC);	//add bidirectionall connectionts from neighbors to q at layer lc
				allNodes[nbr]->Insert(newNodeIndex, lC);

				Neighbours* nbrNbrs = allNodes[nbr]->GetNeighboursAtLayer(lC);
				int eConn = nbrNbrs->Size();

				int MMax_ = MMax;

				if (lC == 0)
					MMax_ = MMax0;

				if (eConn > MMax_)
				{
					Neighbours* eNewCon = new Neighbours(nbrNbrs->layerID);
					eNewCon->neighbours = SelectNeighborsHeuristic(nbr, nbrNbrs->neighbours, MMax_);

					allNodes[nbr]->SetNeighbours(eNewCon);
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

		if (allNodes.size() % 5000 == 0)
			printf("Inserted %d\n", allNodes.size());
	}

	int SearchLayerOne(Node* queryNode, int entryPoint, int lC)
	{
		bool change = true;
		linearHash visitedNodes = linearHash();
		visitedNodes.clear();
		int cNode = entryPoint;
		allNodes[entryPoint]->SetDistance(queryNode);

		while (change)
		{
			change = false;
			vector<unsigned int> nbs = allNodes[cNode]->GetNeighboursVectorAtLayer(lC);

			for (auto n : nbs)
			{
				if (!visitedNodes.get(n))
				{
					visitedNodes.insert(n);
					allNodes[n]->SetDistance(queryNode);

					if (allNodes[n]->distance < allNodes[cNode]->distance)
					{
						change = true;
						cNode = n;
					}
				}
			}
		}

		return cNode;
	}

	vector<int> SearchLayer(Node* queryNode, vector<unsigned int> entryPoints, int K, int layerC)
	{
		SortedNodes nearestNodes = SortedNodes(allNodes, K);
		SortedNodes candidateNodes = SortedNodes(allNodes);
		linearHash visitedNodes = linearHash();
		visitedNodes.clear();

		for (auto p : entryPoints)
		{
			allNodes[p]->SetDistance(queryNode);

			candidateNodes.InsertNode(p);
			nearestNodes.InsertNode(p);
			visitedNodes.insert(p);
		}

		while (candidateNodes.Size() > 0)
		{
			int c = candidateNodes.GetFirstNode();

			if (allNodes[c]->distance > allNodes[nearestNodes.GetLastNode()]->distance)
				break;

			vector<unsigned int> nbs = allNodes[c]->GetNeighboursVectorAtLayer(layerC);
			candidateNodes.RemoveFirstNode();

			for (auto& n : nbs)
			{
				if (!visitedNodes.get(n))
				{
					visitedNodes.insert(n);
					allNodes[n]->SetDistance(queryNode);

					if (allNodes[n]->distance < allNodes[nearestNodes.GetLastNode()]->distance || nearestNodes.Size() < K)
					{
						nearestNodes.InsertNode(n);
						candidateNodes.InsertNode(n);
					}
				}
			}

		}

		return nearestNodes.GetKNearestNodes();
	}

	vector <unsigned int> SearchLayerR(Node* queryNode, vector<unsigned int> entryPoints, int K, int layerC)
	{
		vector<tuple<unsigned int, float>> candidateNodes;
		vector<tuple<unsigned int, float>> nearestNodes;
		linearHash visitedNodes = linearHash();
		visitedNodes.clear();

		float nNDist;
		
		for (auto ep : entryPoints)
		{
			nNDist = allNodes[ep]->SetGetDistance(queryNode);
			candidateNodes.emplace_back(ep, nNDist);
			nearestNodes.emplace_back(ep, nNDist);
			visitedNodes.insert(ep);
		}

		std::make_heap(nearestNodes.begin(), nearestNodes.end(), TupleSortNearest());
		std::make_heap(candidateNodes.begin(), candidateNodes.end(), TupleSortFurthest());

		nNDist = get<1>(nearestNodes.front());
		while (!candidateNodes.empty())
		{
			auto c = candidateNodes.front();

			std::pop_heap(candidateNodes.begin(), candidateNodes.end(), TupleSortFurthest());
			candidateNodes.pop_back();

			if (get<1>(c) > nNDist)
				break;

			for (auto n : allNodes[get<0>(c)]->GetNeighboursVectorAtLayer(layerC))
			{
				if (!visitedNodes.get(n))
				{
					visitedNodes.insert(n);

					float dist = allNodes[n]->SetGetDistance(queryNode);

					if (dist < nNDist || nearestNodes.size() < K)
					{
						candidateNodes.emplace_back(n, dist);
						std::push_heap(candidateNodes.begin(), candidateNodes.end(), TupleSortFurthest());
						nearestNodes.emplace_back(n, dist);
						std::push_heap(nearestNodes.begin(), nearestNodes.end(), TupleSortNearest());

						if (nearestNodes.size() > K)
						{
							std::pop_heap(nearestNodes.begin(), nearestNodes.end(), TupleSortNearest());
							nearestNodes.pop_back();
						}

						nNDist = get<1>(nearestNodes.front());
					}

				}
			}
		}

		vector<unsigned int> finalNearestNodes;
		for (auto n : nearestNodes)
		{
			finalNearestNodes.push_back(get<0>(n));
		}

		return finalNearestNodes;
	}

	vector<int> SearchLayerKNN(Node* queryNode, unsigned int entryPoint, int K, int layerC)
	{
		SortedNodes nearestNodes = SortedNodes(allNodes, K);
		SortedNodes candidateNodes = SortedNodes(allNodes);
		linearHash visitedNodes = linearHash();
		visitedNodes.clear();

		allNodes[entryPoint]->SetDistance(queryNode);

		candidateNodes.InsertNode(entryPoint);
		nearestNodes.InsertNode(entryPoint);
		visitedNodes.insert(entryPoint);

		while (!candidateNodes.Empty())
		{
			int c = candidateNodes.GetFirstNode();

			if (allNodes[c]->distance > allNodes[nearestNodes.GetLastNode()]->distance)
				break;

			vector<unsigned int> nbs = allNodes[c]->GetNeighboursVectorAtLayer(layerC);
			candidateNodes.RemoveFirstNode();


			for (auto n : nbs)
			{
				if(!visitedNodes.get(n))
				{
					visitedNodes.insert(n);
					allNodes[n]->SetDistance(queryNode);

					if (allNodes[n]->distance < allNodes[nearestNodes.GetLastNode()]->distance || nearestNodes.Size() < K)
					{

						nearestNodes.InsertNode(n);
						candidateNodes.InsertNode(n);

					}
				}
			}

		}
		return nearestNodes.GetKNearestNodes();
	}

	vector<tuple<unsigned int, float>> SearchLayerRKNN(Node* queryNode, unsigned int entryPoint, unsigned int K, unsigned int layerC)
	{
		vector<tuple<unsigned int, float>> candidateNodes;
		vector<tuple<unsigned int, float>> nearestNodes;
		linearHash visitedNodes = linearHash();
		visitedNodes.clear();

		float nNDist = allNodes[entryPoint]->SetGetDistance(queryNode);

		candidateNodes.emplace_back(entryPoint, nNDist);
		nearestNodes.emplace_back(entryPoint, nNDist);
		visitedNodes.insert(entryPoint);

		std::make_heap(nearestNodes.begin(), nearestNodes.end(), TupleSortNearest());
		std::make_heap(candidateNodes.begin(), candidateNodes.end(), TupleSortFurthest());

		while (!candidateNodes.empty())
		{
			auto c = candidateNodes.front();

			std::pop_heap(candidateNodes.begin(), candidateNodes.end(), TupleSortFurthest());
			candidateNodes.pop_back();

			if (std::get<1>(c) > nNDist)
				break;

			for (auto n : allNodes[std::get<0>(c)]->GetNeighboursVectorAtLayer(layerC))
			{
				if (!visitedNodes.get(n))
				{
					visitedNodes.insert(n);

					float dist = allNodes[n]->SetGetDistance(queryNode);

					if (dist < nNDist || nearestNodes.size() < K)
					{
						candidateNodes.emplace_back(n, dist);
						std::push_heap(candidateNodes.begin(), candidateNodes.end(), TupleSortFurthest());
						nearestNodes.emplace_back(n, dist);
						std::push_heap(nearestNodes.begin(), nearestNodes.end(), TupleSortNearest());

						if (nearestNodes.size() > K)
						{
							std::pop_heap(nearestNodes.begin(), nearestNodes.end(), TupleSortNearest());
							nearestNodes.pop_back();
						}

						nNDist = get<1>(nearestNodes.front());
					}


				}
			}
		}

		sort(nearestNodes.begin(), nearestNodes.end(), TupleSortNearest());
		return nearestNodes;
	}

	vector<unsigned int> SelectNeighborsSimple(unsigned int queryNode, vector<unsigned int> neighbours, int M)
	{
		if (neighbours.size() <= M)
			return neighbours;

		vector<unsigned int> sortedNodes = neighbours;

		for (auto& n : neighbours)
		{
			allNodes[n]->SetDistance(allNodes[queryNode]);
		}

		sort(sortedNodes.begin(), sortedNodes.end(), NodeDistanceSort(allNodes));
		sortedNodes.erase(sortedNodes.begin() + M, sortedNodes.end());

		return sortedNodes;
	}

	vector<unsigned int> SelectNeighborsHeuristic(unsigned int queryNode, vector<unsigned int> W, int M)
	{
		if (W.size() < M)
		{
			return W;
		}

		for (auto& n : W)
		{
			allNodes[n]->SetDistance(allNodes[queryNode]);
		}

		sort(W.begin(), W.end(), NodeDistanceSort(allNodes));

		vector<unsigned int> R;

		int s = 2;
		int i = 2;

		R.emplace_back(W[0]);
		R.emplace_back(W[1]);

		while (i < W.size() && s < M)
		{
			bool q_is_close = true;
			for (int j = 0; j < R.size(); j++)
			{
				float dist = allNodes[W[i]]->GetDistance(allNodes[R[j]]->values);

				if (dist < allNodes[W[i]]->distance)
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
	/*
	vector<Node*> KNNSearch(Node* queryNode, int K)
	{
		return KNNSearch(queryNode, K, eFConstructions);
	}

	vector<Node*> KNNSearch(Node* queryNode, int K, int efc)
	{
		vector<int> nearestNodesIndex;
		int L = layers.size() - 1;
		int entryPoint = layers[L].entryPoint;

		for (int lC = L; lC >= 1; lC--)
		{
			entryPoint = SearchLayerOne(queryNode, entryPoint, lC);
		}

		nearestNodesIndex = SearchLayerKNN(queryNode, entryPoint, efc, 0);

		vector<Node*> nearestNodes;

		for (int i = 0; i < K; i++)
		{
			nearestNodes.push_back(allNodes[nearestNodesIndex[i]]);
		}

		return nearestNodes;
	}
	*/
	vector<unsigned int> KNNSearchIndex(Node* queryNode, int K)
	{
		return KNNSearchIndex(queryNode, K, eFConstructions);
	}

	vector<unsigned int> KNNSearchIndex(Node* queryNode, int K, int efC)
	{
		int L = layers.size() - 1;
		unsigned int entryPoint = layers[L].entryPoint;

		for (int lC = L; lC >= 1; lC--)
		{
			entryPoint = SearchLayerOne(queryNode, entryPoint, lC);
		}

		
		vector<unsigned int> nearestNodes;
		vector<tuple<unsigned int, float>> nearestNodesIndexRef = SearchLayerRKNN(queryNode, entryPoint, efC, 0);

		for (int i = 0; i < K; i++)
		{
			nearestNodes.push_back(get<0>(nearestNodesIndexRef[i]));
		}

		return nearestNodes;
	}

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

			vector<unsigned int> nbs = allNodes[i]->GetNeighboursVectorAtLayer(0);

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

			vector<unsigned int> nbs = allNodes[i]->GetNeighboursVectorAtLayer(0);

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

