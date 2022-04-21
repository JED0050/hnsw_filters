#pragma once

#include "Hnsw.h"

unsigned int Node::vectorSize = 128;

Hnsw::Hnsw()
{
	M = 16;
	MMax = 16;
	eFConstructions = 200;

	MMax0 = MMax * 2;
	mL = 1 / log(0.8 * M);
}

Hnsw::Hnsw(int M, int MMax, int eFC)
{
	this->M = M;
	this->MMax = MMax;
	this->eFConstructions = eFC;

	this->MMax0 = MMax * 2;
	this->mL = 1 / log(0.8 * M);
}

Hnsw::~Hnsw()
{
	/*for (auto n : allNodes)
	{
		delete n;
	}*/
	//allNodes.clear();
}


/////////////////////////////// INSERT NODE ///////////////////////////////

void Hnsw::Insert(Node* newNode)
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

void Hnsw::SearchLayerOne(Node* queryNode, NodeDist& W, int lC)
{
	bool change = true;
	linearHash V = linearHash();
	V.clear();
	V.insert(W.ID);

	while (change)
	{
		change = false;
		vector<uint> nbs = allNodes[W.ID]->GetNeighboursVectorAtLayer(lC);

		for (auto n : nbs)
		{
			if (!V.get(n))
			{
				V.insert(n);
				NodeDist newNode = NodeDist(n);
				newNode.SetDistance(allNodes[n]->values, queryNode->values);

				if (newNode.distance < W.distance)
				{
					change = true;
					W = newNode;
				}
			}
		}
	}
}

vector<NodeDist> Hnsw::SearchLayer(Node* queryNode, vector<NodeDist> W, int K, int layerC)
{
	vector<NodeDist> C = W;
	linearHash V = linearHash();
	V.clear();

	float nNDist;

	for (auto ep : W)
	{
		V.insert(ep.ID);
	}

	make_heap(W.begin(), W.end(), NodeDistanceSortNearest());
	make_heap(C.begin(), C.end(), NodeDistanceSortFurthest());

	nNDist = W.front().distance;
	while (!C.empty())
	{
		auto c = C.front();

		pop_heap(C.begin(), C.end(), NodeDistanceSortFurthest());
		C.pop_back();

		if (c.distance > nNDist)
			break;

		for (auto n : allNodes[c.ID]->GetNeighboursVectorAtLayer(layerC))
		{
			if (!V.get(n))
			{
				V.insert(n);

				NodeDist newNode = NodeDist(n);
				newNode.SetDistance(allNodes[n]->values, queryNode->values);

				if (newNode.distance < nNDist || W.size() < K)
				{
					C.push_back(newNode);
					push_heap(C.begin(), C.end(), NodeDistanceSortFurthest());
					W.push_back(newNode);
					push_heap(W.begin(), W.end(), NodeDistanceSortNearest());

					if (W.size() > K)
					{
						pop_heap(W.begin(), W.end(), NodeDistanceSortNearest());
						W.pop_back();
					}

					nNDist = W.front().distance;
				}

			}
		}
	}

	return W;
}

vector<NodeDist> Hnsw::SelectNeighborsSimple(uint queryNode, vector<NodeDist> W, int M)
{
	if (W.size() <= M)
		return W;

	for (auto& n : W)
	{
		n.SetDistance(allNodes[n.ID]->values, allNodes[queryNode]->values);
	}

	sort(W.begin(), W.end(), NodeDistanceSortNearest());
	W.erase(W.begin() + M, W.end());

	return W;
}

vector<NodeDist> Hnsw::SelectNeighborsHeuristic(uint queryNode, vector<NodeDist> W, int M, bool computeDist)
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

vector<NodeDist> Hnsw::SearchLayerKNN(Node* queryNode, NodeDist& We, uint K)
{
	vector<NodeDist> C;
	vector<NodeDist> W;

	//uint hashSize = 1048576;

	uint hashSize = 16384;			//2^14

	if (K > 300)
		hashSize = 1048576;			//2^20

	/*if (K > 1000 && K < 10000)
		hashSize = 65536;			//2^16
	else if (K >= 10000)
		hashSize = 262144;			//2^18
	else if (K >= 100000)
		hashSize = 1048576;			//2^20*/

	linearHash V = linearHash(hashSize);

	V.clear();

	float nNDist = We.distance;

	C.push_back(We);
	W.push_back(We);
	V.insert(We.ID);

	make_heap(W.begin(), W.end(), NodeDistanceSortNearest());
	make_heap(C.begin(), C.end(), NodeDistanceSortFurthest());

	while (!C.empty())
	{
		auto c = C.front();

		pop_heap(C.begin(), C.end(), NodeDistanceSortFurthest());
		C.pop_back();

		if (c.distance > nNDist)
			break;

		//cout << "For: " << allNodes[c.ID]->GetNeighboursVectorAtLayer(0).size() << endl;

		for (auto n : allNodes[c.ID]->GetNeighboursVectorAtLayer(0))
		{
			//cout << "Is visited?" << endl;
			if (!V.get(n))
			{
				//cout << "New visited: " << n << endl;

				V.insert(n);

				NodeDist newNode = NodeDist(n);
				newNode.SetDistance(allNodes[n]->values, queryNode->values);

				if (newNode.distance < nNDist || W.size() < K)
				{
					//cout << "Is closer " << endl;

					C.push_back(newNode);
					push_heap(C.begin(), C.end(), NodeDistanceSortFurthest());
					W.push_back(newNode);
					push_heap(W.begin(), W.end(), NodeDistanceSortNearest());

					if (W.size() > K)
					{
						pop_heap(W.begin(), W.end(), NodeDistanceSortNearest());
						W.pop_back();
					}

					nNDist = W.front().distance;
				}


			}
			//else
				//cout << "Was visited!" << endl;
		}
	}

	//cout << "Almost:" << nearestNodes.size() << endl;

	sort(W.begin(), W.end(), NodeDistanceSortNearest());
	return W;
}

vector<uint> Hnsw::KNN(Node* queryNode, int K, int efC)
{
	int L = layers.size() - 1;
	NodeDist W = NodeDist(layers[L].entryPoint);
	W.SetDistance(allNodes[W.ID]->values, queryNode->values);

	for (int lC = L; lC >= 1; lC--)
	{
		SearchLayerOne(queryNode, W, lC);
	}

	vector<uint> nearestNodes;
	vector<NodeDist> nearestNodesIndexRef = SearchLayerKNN(queryNode, W, efC);

	for (int i = 0; i < K; i++)
	{
		nearestNodes.push_back(nearestNodesIndexRef[i].ID);
	}

	return nearestNodes;
}

/////////////////////////////// SEARCH FILTER ///////////////////////////////

void Hnsw::SearchLayerOneFilter(Node* queryNode, NodeDist& W, vector<DimFilter> filters, int lC)
{
	bool change = true;
	linearHash V = linearHash();
	V.clear();
	V.insert(W.ID);

	while (change)
	{
		change = false;
		vector<uint> nbs = allNodes[W.ID]->GetNeighboursVectorAtLayer(lC);

		for (auto n : nbs)
		{
			if (!V.get(n))
			{
				V.insert(n);

				NodeDist newNode = NodeDist(n);
				newNode.SetDistance(allNodes[n]->values, queryNode->values);

				if (newNode.distance < W.distance)
				{
					if (DimFilterHelper::IsVectorValid(filters, allNodes[n]->values))
					{
						change = true;
						W = newNode;
					}
				}
			}
		}
	}
}

vector<NodeDist> Hnsw::SearchLayerFilter(Node* queryNode, NodeDist& We, vector<DimFilter> filters, uint K, uint KNN)
{
	vector<NodeDist> C;
	vector<NodeDist> W;

	uint hashSize = 16384;			//2^14
	if (K >= 100)
		hashSize = 262144;			//2^18
	linearHash V = linearHash(hashSize);
	V.clear();

	float nNDist = We.distance;

	C.push_back(We);
	W.push_back(We);
	V.insert(We.ID);

	make_heap(W.begin(), W.end(), NodeDistanceSortNearest());
	make_heap(C.begin(), C.end(), NodeDistanceSortFurthest());

	vector<NodeDist> F;
	if (DimFilterHelper::IsVectorValid(filters, allNodes[We.ID]->values))
	{
		F.push_back(We);
	}
	make_heap(F.begin(), F.end(), NodeDistanceSortNearest());

	while (!C.empty())
	{
		auto c = C.front();

		pop_heap(C.begin(), C.end(), NodeDistanceSortFurthest());
		C.pop_back();

		if (c.distance > nNDist && F.size() == KNN)
			break;

		for (auto n : allNodes[c.ID]->GetNeighboursVectorAtLayer(0))
		{
			if (!V.get(n))
			{
				V.insert(n);

				NodeDist newNode = NodeDist(n);
				newNode.SetDistance(allNodes[n]->values, queryNode->values);

				if (F.size() < KNN || newNode.distance < F.front().distance)
				{
					if (DimFilterHelper::IsVectorValid(filters, allNodes[n]->values))
					{
						F.push_back(newNode);
						push_heap(F.begin(), F.end(), NodeDistanceSortNearest());

						if (F.size() > KNN)
						{
							pop_heap(F.begin(), F.end(), NodeDistanceSortNearest());
							F.pop_back();
						}
					}
				}

				if (newNode.distance < nNDist || W.size() < K)
				{
					C.push_back(newNode);
					push_heap(C.begin(), C.end(), NodeDistanceSortFurthest());

					W.push_back(newNode);
					push_heap(W.begin(), W.end(), NodeDistanceSortNearest());

					if (W.size() > K)
					{
						pop_heap(W.begin(), W.end(), NodeDistanceSortNearest());
						W.pop_back();
					}

					nNDist = W.front().distance;
				}
			}
		}
	}

	sort(F.begin(), F.end(), NodeDistanceSortNearest());
	return F;
}

vector<uint> Hnsw::KNNFilter(Node* queryNode, vector<DimFilter> filters, int K, int efC)
{
	int L = layers.size() - 1;
	NodeDist W = NodeDist(layers[L].entryPoint);
	W.SetDistance(allNodes[W.ID]->values, queryNode->values);

	for (int lC = L; lC >= 1; lC--)
	{
		SearchLayerOne(queryNode, W, lC);
		//SearchLayerOneFilter(queryNode, W, filters, lC);
	}

	vector<uint> nearestNodes;
	vector<NodeDist> nearestNodesIndexRef = SearchLayerFilter(queryNode, W, filters, efC, K);

	//if (nearestNodesIndexRef.size() < K)
	//	cout << "Found only " << nearestNodesIndexRef.size() << "/" << K << " nodes" << endl;

	for (int i = 0; i < nearestNodesIndexRef.size(); i++)
	{
		nearestNodes.push_back(nearestNodesIndexRef[i].ID);
	}

	return nearestNodes;
}


/////////////////////////////// STATISTICS ///////////////////////////////

void Hnsw::PrintInfo(int n)
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

void Hnsw::PrintInfoSorted(int n)
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

void Hnsw::SavePrint(int max_count, string name)
{
	if (max_count == 0)
		max_count = allNodes.size();

	ofstream MyFile(name);

	for (int i = 0; i < max_count; i++) {
		if (i % 5000 == 0)
		{
			cout << i << " / " << max_count << endl;
		}

		string line = to_string(i) + ": ";

		vector<uint> nbs = allNodes[i]->GetNeighboursVectorAtLayer(0);

		sort(nbs.begin(), nbs.end());

		for (auto& n : nbs)
		{
			line += to_string(n) + "  ";
		}

		line += "\n";

		MyFile << line;
	}

	MyFile.close();
}