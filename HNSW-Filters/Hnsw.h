#pragma once
#include<vector>
#include"Node.h"
#include"Layer.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class Hnsw
{
public:
	Node* mainEntryPoint;
	vector<Layer*> layers;

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

	void Insert(Node *newNode)
	{
		allNodes.push_back(newNode);

		int newNodeIndex = allNodes.size() - 1;

		if (mainEntryPoint == nullptr)
		{
			mainEntryPoint = allNodes[0];
			Layer* newLayer = new Layer(0);
			layers.push_back(newLayer);

			return;
		}

		vector<int> nearestNodes;
		int entryPoint = 0;
		int L = layers.size() - 1;
		int l = -log(((float)(rand() % 10000 + 1)) / 10000) * mL; //(0 - 9) * mL => 0 - 3

		for (int lC = L; lC >= l + 1; lC--)
		{
			entryPoint = SearchLayer(*newNode, entryPoint, 1, lC)[0];
		}

		for (int lC = min(L, l); lC >= 0; lC--)
		{
			
			vector<int> W = SearchLayer(*newNode, entryPoint, eFConstructions, lC);
			vector<int> neighbors = SelectNeighborsHeuristic(newNodeIndex, W, M); //SelectNeighborsHeuristic(W, nbrNbrs, MMax_);

			/*
			if (W.size() > M)
				neighbors.assign(W.begin(), W.end() - (W.size() - M));
			else
				neighbors = W;*/


			for (auto &nbr : neighbors)
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
					//Neighbours* eNewCon = SelectNeighborsSimple(nbr, nbrNbrs, MMax_);
					Neighbours* eNewCon = new Neighbours(nbrNbrs->layerID);
					eNewCon->neighbours = SelectNeighborsHeuristic(nbr, nbrNbrs->neighbours, MMax_);

					allNodes[nbr]->SetNeighbours(eNewCon);
				}
			}

			entryPoint = W[0];
		}

		while (l > L)
		{
			Layer* newLayer = new Layer(entryPoint);
			layers.push_back(newLayer);

			l--;
		}

		if(allNodes.size() % 1000 == 0)
			printf("Inserted %d\n", allNodes.size());

		/*
		if (allNodes.size() == 1000)
		{
			int c = 0;

			printf("Size: %d\n", allNodes.size());

			for (auto& n : allNodes)
			{
				printf("c: %d\n", c);

				for (auto& l : n->lNaighbours)
				{
					printf("\t l: %d s: %d\n\t\t", l->layerID, l->Size());

					for (auto& i : l->neighbours)
					{
						printf("%d ", i);
					}

					printf("\n");
				}
					
				c++;
			}

		}*/
	}

	vector<int> SearchLayer(Node queryNode, int entryPoint, int K, int layerC)
	{
		SortedNodes nearestNodes = SortedNodes(allNodes, K);
		SortedNodes candidateNodes = SortedNodes(allNodes);
		vector<int> visitedNodes;
		
		allNodes[entryPoint]->SetDistance(queryNode);

		candidateNodes.InsertNode(entryPoint);
		nearestNodes.InsertNode(entryPoint);
		visitedNodes.push_back(entryPoint);

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

	vector<int> SelectNeighborsSimple(int queryNode, vector<int> neighbours, int M)
	{
		if (neighbours.size() <= M)
			return neighbours;

		vector<int> sortedNodes = neighbours;

		for (auto& n : neighbours)
		{
			allNodes[n]->SetDistance(*allNodes[queryNode]);
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
			allNodes[n]->SetDistance(*allNodes[queryNode]);
		}

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
		}

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
				float dist = allNodes[W[i]]->GetDistance(*allNodes[R[j]]); 

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

	vector<Node*> KNNSearch(Node queryNode, int K)
	{
		vector<int> nearestNodesIndex;
		int L = layers.size() - 1;
		int entryPoint = layers[L]->entryPoint;

		for (int lC = L; lC >= 1; lC--)
		{
			nearestNodesIndex = SearchLayer(queryNode, entryPoint, 1, lC);
			entryPoint = nearestNodesIndex[0];
		}

		nearestNodesIndex = SearchLayer(queryNode, entryPoint, eFConstructions, 0);
		//nearestNodesIndex = SearchLayer(queryNode, entryPoint, K, 0);

		vector<Node*> nearestNodes;

		for (int i = 0; i < K; i++)
		{
			nearestNodes.push_back(allNodes[nearestNodesIndex[i]]);
		}

		return nearestNodes;
	}

	vector<int> KNNSearchIndex(Node queryNode, int K)
	{
		vector<int> nearestNodesIndex;
		int L = layers.size() - 1;
		int entryPoint = layers[L]->entryPoint;

		for (int lC = L; lC >= 1; lC--)
		{
			nearestNodesIndex = SearchLayer(queryNode, entryPoint, 1, lC);
			entryPoint = nearestNodesIndex[0];
		}

		nearestNodesIndex = SearchLayer(queryNode, entryPoint, eFConstructions, 0);
		//nearestNodesIndex = SearchLayer(queryNode, entryPoint, K, 0);

		vector<int> nearestNodes;

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
				printf("%d ", n);
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
			std::cout << i << " / " << max_count << std::endl;

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

