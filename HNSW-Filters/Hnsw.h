#pragma once
#include<vector>
#include"Node.h"
#include"Layer.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <set>

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

		int newNodeIndex = allNodes.size() - 1;
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

		int entryPoint = layers[L].entryPoint;

		for (int lC = L; lC > l; lC--)
		{
			entryPoint = SearchLayerOne(newNode, entryPoint, lC);
		}

		vector<int> W;
		W.push_back(entryPoint);

		for (int lC = min(L, l); lC >= 0; lC--)
		{
			W = SearchLayer(newNode, W, eFConstructions, lC);

			vector<int> neighbors = SelectNeighborsHeuristic(newNodeIndex, W, M);

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
		vector<int> visitedNodes;
		int cNode = entryPoint;
		allNodes[entryPoint]->SetDistance(queryNode);

		while (change)
		{
			change = false;

			vector<int> nbs = allNodes[cNode]->GetNeighboursVectorAtLayer(lC);

			for (auto n : nbs)
			{
				bool visited = false;

				for (auto vn : visitedNodes)
				{
					if (vn == n)
					{
						visited = true;
						break;
					}
				}

				if (!visited)
				{
					allNodes[n]->SetDistance(queryNode);
					visitedNodes.push_back(n);

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

	vector<int> SearchLayer(Node* queryNode, vector<int> entryPoints, int K, int layerC)
	{
		SortedNodes nearestNodes = SortedNodes(allNodes, K);
		SortedNodes candidateNodes = SortedNodes(allNodes);
		vector<int> visitedNodes;

		for (auto p : entryPoints)
		{
			allNodes[p]->SetDistance(queryNode);

			candidateNodes.InsertNode(p);
			nearestNodes.InsertNode(p);
			visitedNodes.push_back(p);
		}

		while (candidateNodes.Size() > 0)
		{
			int c = candidateNodes.GetFirstNode();

			if (allNodes[c]->distance > allNodes[nearestNodes.GetLastNode()]->distance)
				break;

			vector<int> nbs = allNodes[c]->GetNeighboursVectorAtLayer(layerC);
			candidateNodes.RemoveFirstNode();

			for (auto& n : nbs)
			{
				bool nodeVisited = false;

				for (auto& vn : visitedNodes)
				{
					if (vn == n)
					{
						nodeVisited = true;
						break;
					}
				}

				if (!nodeVisited)
				{
					visitedNodes.push_back(n);
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

	vector<int> SearchLayerKNN(Node* queryNode, int entryPoint, int K, int layerC)
	{
		SortedNodes nearestNodes = SortedNodes(allNodes, K);
		SortedNodes candidateNodes = SortedNodes(allNodes);
		vector<int> visitedNodes;

		allNodes[entryPoint]->SetDistance(queryNode);

		candidateNodes.InsertNode(entryPoint);
		nearestNodes.InsertNode(entryPoint);
		visitedNodes.push_back(entryPoint);

		while (!candidateNodes.Empty())
		{

			int c = candidateNodes.GetFirstNode();

			if (allNodes[c]->distance > allNodes[nearestNodes.GetLastNode()]->distance)
				break;

			vector<int> nbs = allNodes[c]->GetNeighboursVectorAtLayer(layerC);
			candidateNodes.RemoveFirstNode();

			for (auto n : nbs)
			{
				bool nodeVisited = false;

				for (auto vn : visitedNodes)
				{
					if (vn == n)
					{
						nodeVisited = true;
						break;
					}
				}

				if (!nodeVisited)
				{
					visitedNodes.push_back(n);
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

	vector<tuple<unsigned int, float>> SearchLayerKNNTuple(Node* queryNode, unsigned int entryPoint, int K, int layerC)
	{
		SortedNodesTuple nearestNodes = SortedNodesTuple(K);
		SortedNodesTuple candidateNodes = SortedNodesTuple();
		vector<int> visitedNodes;

		//allNodes[entryPoint]->SetDistance(queryNode);
		float nDist = allNodes[entryPoint]->SetGetDistance(queryNode);

		tuple<unsigned int, float> entryNode = make_tuple(entryPoint, nDist);

		candidateNodes.InsertNode(entryNode);
		nearestNodes.InsertNode(entryNode);
		visitedNodes.push_back(entryPoint);


		while (!candidateNodes.Empty())
		{

			int c = candidateNodes.GetFirstNode();
			float cDist = allNodes[c]->distance;

			if (cDist > nDist)
				break;

			vector<int> nbs = allNodes[c]->GetNeighboursVectorAtLayer(layerC);
			candidateNodes.RemoveFirstNode();

			for (auto n : nbs)
			{
				bool nodeVisited = false;

				for (auto vn : visitedNodes)
				{
					if (vn == n)
					{
						nodeVisited = true;
						break;
					}
				}

				if (!nodeVisited)
				{
					visitedNodes.push_back(n);
					float newDist = allNodes[n]->SetGetDistance(queryNode);

					if (newDist < nDist || nearestNodes.Size() < K)
					{
						tuple<unsigned int, float> newNode = make_tuple(n, newDist);

						nearestNodes.InsertNode(newNode);
						candidateNodes.InsertNode(newNode);

						nDist = allNodes[nearestNodes.GetLastNode()]->distance;
					}
				}
			}

		}

		return nearestNodes.GetKNearestNodesSorted();
	}

	vector<int> SearchLayerN(Node* queryNode, int entryPoint, int K, int layerC)
	{
		vector<int> nearestNodes;
		vector<int> candidateNodes;
		//vector<int> visitedNodes;
		std::set<int> visitedNodes;

		allNodes[entryPoint]->SetDistance(queryNode);

		candidateNodes.push_back(entryPoint);
		nearestNodes.push_back(entryPoint);
		//visitedNodes.push_back(entryPoint);
		visitedNodes.insert(entryPoint);

		//int cMinNode = entryPoint;
		int nMaxNode = entryPoint;

		while (!candidateNodes.empty())
		{
			int cMinNode = candidateNodes[0];
			sort(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSort(allNodes));
			candidateNodes.erase(candidateNodes.begin());

			if (allNodes[cMinNode]->distance > allNodes[nMaxNode]->distance)
				break;

			//vector<int> nbs = allNodes[cMinNode]->GetNeighboursVectorAtLayer(layerC);
			//candidateNodes.erase(std::remove(candidateNodes.begin(), candidateNodes.end(), cMinNode), candidateNodes.end());

			for (auto& n : allNodes[cMinNode]->GetNeighboursVectorAtLayer(layerC))
			{

				if (visitedNodes.empty() || !(visitedNodes.find(n) != visitedNodes.end()))
				{
					visitedNodes.insert(n);
					allNodes[n]->SetDistance(queryNode);

					if (allNodes[n]->distance < allNodes[nMaxNode]->distance || nearestNodes.size() < K)
					{
						candidateNodes.push_back(n);
						sort(candidateNodes.begin(), candidateNodes.end(), NodeDistanceSort(allNodes));

						nearestNodes.push_back(n);
						sort(nearestNodes.begin(), nearestNodes.end(), NodeDistanceSort(allNodes));

						if (nearestNodes.size() > K)
						{
							nearestNodes.pop_back();
						}

						nMaxNode = nearestNodes[nearestNodes.size() - 1];
					}
				}
			}

		}

		return nearestNodes;
	}

	vector<tuple<unsigned int, float>> SearchLayerR(Node* queryNode, unsigned int entryPoint, unsigned int K, unsigned int layerC)
	{
		vector<tuple<unsigned int, float>> candidateNodes;
		vector<tuple<unsigned int, float>> nearestNodes;
		vector<int> visitedNodes;
		//linearHash visitedNodes;

		allNodes[entryPoint]->SetDistance(queryNode);
		float nNDist = allNodes[entryPoint]->distance;

		candidateNodes.emplace_back(entryPoint, nNDist);
		nearestNodes.emplace_back(entryPoint, nNDist);
		visitedNodes.emplace_back(entryPoint);
		//visitedNodes.insert(entryPoint);

		/*
		for (auto n : entryPoints)
		{
			candidateNodes.emplace_back(n.node, n.distance);
			visited_.insert(n.node_order);
		}*/

		std::make_heap(nearestNodes.begin(), nearestNodes.end(), TupleSortNearest());
		std::make_heap(candidateNodes.begin(), candidateNodes.end(), TupleSortFurthest());

		while (!candidateNodes.empty())
		{
			auto c = candidateNodes.front();

			std::pop_heap(candidateNodes.begin(), candidateNodes.end(), TupleSortFurthest());
			candidateNodes.pop_back();

			if (std::get<1>(c) > nNDist)
				break;

			for (auto ne : allNodes[std::get<0>(c)]->GetNeighboursVectorAtLayer(layerC))
			{
				//auto e = ne.node;

				//if (!visitedNodes.get(ne))
				if(!(std::find(visitedNodes.begin(), visitedNodes.end(), ne) != visitedNodes.end()))
				{
					visitedNodes.push_back(ne);
					//visitedNodes.insert(ne);

					allNodes[ne]->SetDistance(queryNode);
					float dist = allNodes[ne]->distance;

					if (dist < nNDist || nearestNodes.size() < K)
					{
						candidateNodes.emplace_back(ne, dist);
						std::push_heap(candidateNodes.begin(), candidateNodes.end(), TupleSortFurthest());
						nearestNodes.emplace_back(ne, dist);
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

		return nearestNodes;
	}

	vector<int> SelectNeighborsSimple(int queryNode, vector<int> neighbours, int M)
	{
		if (neighbours.size() <= M)
			return neighbours;

		vector<int> sortedNodes = neighbours;

		for (auto& n : neighbours)
		{
			allNodes[n]->SetDistance(allNodes[queryNode]);
		}


		bool changed = true;

		while (changed)
		{
			changed = false;

			for (int i = 0; i < sortedNodes.size() - 1; i++)
			{
				if (allNodes[sortedNodes[i]]->distance > allNodes[sortedNodes[i + 1]]->distance)
				{
					changed = true;
					int tmpNode = sortedNodes[i + 1];
					sortedNodes[i + 1] = sortedNodes[i];
					sortedNodes[i] = tmpNode;
				}
			}
		}

		sortedNodes.erase(sortedNodes.begin() + M, sortedNodes.end());

		//neighbours->neighbours = sortedNodes;
		//return neighbours;

		return sortedNodes;
	}

	vector<int> SelectNeighborsHeuristic(int queryNode, vector<int> W, int M)
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

		/*
		bool changed = true;

		while (changed)
		{
			changed = false;

			for (int i = 0; i < W.size() - 1; i++)
			{
				if (allNodes[W[i]]->distance > allNodes[W[i + 1]]->distance)
				{
					changed = true;
					int tmpNode = W[i + 1];
					W[i + 1] = W[i];
					W[i] = tmpNode;
				}
			}
		}*/

		vector<int> R;

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
	vector<int> KNNSearchIndex(Node* queryNode, int K)
	{
		return KNNSearchIndex(queryNode, K, eFConstructions);
	}

	vector<int> KNNSearchIndex(Node* queryNode, int K, int efC)
	{
		int L = layers.size() - 1;
		int entryPoint = layers[L].entryPoint;

		for (int lC = L; lC >= 1; lC--)
		{
			entryPoint = SearchLayerOne(queryNode, entryPoint, lC);
		}

		vector<int> nearestNodes;

		vector<int> nearestNodesIndex = SearchLayerKNN(queryNode, entryPoint, efC, 0);

		for (int i = 0; i < K; i++)
		{
			nearestNodes.push_back(nearestNodesIndex[i]);
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

			vector<int> nbs = allNodes[i]->GetNeighboursVectorAtLayer(0);

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

			vector<int> nbs = allNodes[i]->GetNeighboursVectorAtLayer(0);

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

