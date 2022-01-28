#pragma once
#include <vector>
#include <algorithm>
#include<tuple>

using namespace std;

class Node;
class Neighbours
{
public:
	int layerID;
	vector<unsigned int> neighbours;

	Neighbours(int lID)
	{
		layerID = lID;
	}

	void Insert(unsigned int newNode)
	{
		neighbours.push_back(newNode);
	}

	int Size()
	{
		return neighbours.size();
	}

	vector<unsigned int> GetNeighbours()
	{
		return neighbours;
	}

};


class Node
{
public:
	static unsigned int vectorSize;
	//float values[128];
	vector<float> values;
	vector<Neighbours*> lNaighbours;
	float distance = -1;

	//~Node()
	//{
	//	for (auto& n : neighbours)
	//	{
	//		delete n;
	//	}
	//}

	void Insert(int node, int layerID)
	{
		if (lNaighbours.size() == 0)
		{
			Neighbours* newNeighbours = new Neighbours(layerID);
			newNeighbours->Insert(node);
			lNaighbours.push_back(newNeighbours);

		}
		else
		{
			for (auto& nbr : lNaighbours)
			{
				if (nbr->layerID == layerID)
				{
					nbr->Insert(node);
					return;
				}
			}

			Neighbours* newNeighbours = new Neighbours(layerID);
			newNeighbours->Insert(node);
			lNaighbours.push_back(newNeighbours);
		}
	}

	void SetNeighbours(Neighbours* nbrs)
	{
		for (auto nbr : lNaighbours)
		{
			if (nbr->layerID == nbrs->layerID)
			{
				nbr->neighbours = nbrs->neighbours;
				return;
			}
		}

		lNaighbours.push_back(nbrs);
	}

	Neighbours* GetNeighboursAtLayer(int layerID)
	{
		for (auto& nbr : lNaighbours)
		{
			if (nbr->layerID == layerID)
			{
				return nbr;
			}
		}

		return new Neighbours(layerID);
	}

	vector<unsigned int> GetNeighboursVectorAtLayer(int layerID)
	{
		for (auto& nbr : lNaighbours)
		{
			if (nbr->layerID == layerID)
			{
				return nbr->neighbours;
			}
		}

		vector<unsigned int> emptyVector;
		return emptyVector;
	}

	vector<int> GetNeighboursVectorAll()
	{
		vector<int> nbs;

		for (auto& nbr : lNaighbours)
		{
			for (auto n : nbr->neighbours)
			{
				bool cont = false;

				for (auto c : nbs)
				{
					if (c == n)
					{
						cont = true;
						break;
					}
				}

				if (!cont)
				{
					nbs.push_back(n);
				}
			}
		}

		return nbs;
	}

	void InsertValue(float v, int index)
	{
		values[index] = v;
	}

	void SetDistance(Node* node)
	{
		distance = GetDistance(node->values);
	}

	float GetDistance(vector<float> node)
	{
		float distance = 0;

		for (int i = 0; i < vectorSize; i++)
		{
			float x = node[i];
			float y = values[i];
			float z = x - y;

			distance += (z * z);
		}

		return distance;
		//return sqrt(distance);
	}

	float SetGetDistance(Node* node)
	{
		distance = GetDistance(node->values);
		return distance;
	}

};

struct NodeDistanceSort
{
	vector<Node*> allNodes;

	NodeDistanceSort(vector<Node*>& aN)
	{
		allNodes = aN;
	}

	inline bool operator() (const int& left, const int& right)
	{
		return (allNodes[left]->distance < allNodes[right]->distance);
	}
};

struct TupleSortNearest {
	bool operator()(const tuple<unsigned int, float>& a, const tuple<unsigned int, float>& b) const {
		return get<1>(a) < get<1>(b);
	}
};

struct TupleSortFurthest {
	constexpr bool operator()(std::tuple<unsigned int, float> i1, std::tuple<unsigned int, float> i2) const noexcept
	{
		return std::get<1>(i1) > std::get<1>(i2);
	}
};

class SortedNodes
{
public:
	vector<int> nodes;
	int K;

	int minI;
	int maxI;

	int minV;
	int maxV;

	vector<Node*> hnswNodes;

	SortedNodes(vector<Node*>& allNodes)
	{
		hnswNodes = allNodes;
		K = -1;
	}

	SortedNodes(vector<Node*>& allNodes, int K)
	{
		hnswNodes = allNodes;
		this->K = K;
	}

	void InsertNode(int node)
	{
		if (K == -1 || nodes.size() < K)
		{
			nodes.push_back(node);

			//Sort();

			int nodeLastIndex = nodes.size() - 1;

			if (nodeLastIndex + 1 == 1)
			{
				minV = node;
				maxV = node;

				minI = nodeLastIndex;
				maxI = nodeLastIndex;
			}
			else
			{
				if (hnswNodes[node]->distance < hnswNodes[minV]->distance)
				{
					minV = node;
					minI = nodeLastIndex;
				}

				if (hnswNodes[node]->distance > hnswNodes[maxV]->distance)
				{
					maxV = node;
					maxI = nodeLastIndex;
				}
			}

		}
		else if (hnswNodes[maxV]->distance > hnswNodes[node]->distance)
		{
			nodes[maxI] = node;
			maxV = node;

			//Sort();

			if (hnswNodes[node]->distance < hnswNodes[minV]->distance)
			{
				minI = maxI;
				minV = node;
			}

			for (int i = 0; i < nodes.size(); i++)
			{
				if (hnswNodes[nodes[i]]->distance > hnswNodes[maxV]->distance)
				{
					maxV = nodes[i];
					maxI = i;
				}
			}
		}
	}

	int GetFirstNode()
	{
		//Sort();

		return minV;
	}

	int GetLastNode()
	{
		//Sort();

		return maxV;
	}

	void RemoveFirstNode()
	{
		if (nodes.size() <= 1)
		{
			nodes.clear();

			minI = -1;
			maxI = -1;
			minV = -1;
			maxV = -1;

			return;
		}

		//Sort();

		nodes.erase(nodes.begin() + minI);

		minI = 0;
		minV = nodes[minI];

		for (int i = 0; i < nodes.size(); i++)
		{
			if (hnswNodes[nodes[i]]->distance < hnswNodes[minV]->distance)
			{
				minV = nodes[i];
				minI = i;
			}
		}
	}

	int Size()
	{
		return nodes.size();
	}

	bool Empty()
	{
		return nodes.empty();
	}

	void Sort()
	{
		sort(nodes.begin(), nodes.end(), NodeDistanceSort(hnswNodes));

		minV = nodes[0];
		minI = 0;
		maxV = nodes[Size() - 1];
		maxI = Size() - 1;
	}

	void Print()
	{
		for (auto& n : nodes)
			printf("%d ", n);
		printf("\n");
	}

	vector<int> GetKNearestNodes()
	{
		return nodes;
	}

	vector<int> GetKNearestNodesSorted()
	{
		Sort();
		return nodes;
	}
};

class SortedNodesO
{
public:
	vector<int> nodes;
	int K;

	vector<Node*> hnswNodes;

	SortedNodesO(vector<Node*>& allNodes)
	{
		hnswNodes = allNodes;
		K = -1;
	}

	SortedNodesO(vector<Node*>& allNodes, int K)
	{
		hnswNodes = allNodes;
		this->K = K;
	}

	void InsertNode(int node)
	{
		if (K == -1 || nodes.size() < K)
		{
			nodes.push_back(node);
		}
		else if (hnswNodes[nodes.size() - 1]->distance > hnswNodes[node]->distance)
		{
			nodes[nodes.size() - 1] = node;
		}

		Sort();
	}

	int GetFirstNode()
	{
		return nodes[0];
	}

	int GetLastNode()
	{
		return nodes[nodes.size() - 1];
	}

	void RemoveFirstNode()
	{
		if (nodes.size() <= 1)
		{
			nodes.clear();
			return;
		}

		nodes.erase(nodes.begin());
	}

	int Size()
	{
		return nodes.size();
	}

	bool Empty()
	{
		return nodes.empty();
	}

	void Sort()
	{
		sort(nodes.begin(), nodes.end(), NodeDistanceSort(hnswNodes));
	}

	vector<int> GetKNearestNodes()
	{
		return nodes;
	}

	vector<int> GetKNearestNodesSorted()
	{
		Sort();
		return nodes;
	}
};

class linearHash
{
	uint32_t actual_size;
	uint32_t mask;
	uint32_t* hasharray;

public:

	uint32_t item_count;

	linearHash(uint32_t asize = 16384)
	{
		item_count = 0;
		actual_size = asize;
		mask = asize - 1;
		hasharray = new uint32_t[actual_size];
	}

	~linearHash()
	{
		delete[] hasharray;
	}

	void clear()
	{
		item_count = 0;
		memset(hasharray, -1, sizeof(uint32_t) * actual_size);
	}

	void reduce(int shift)
	{
		actual_size = actual_size >> shift;
		mask = actual_size - 1;
	}

	void insert(uint32_t index)
	{
		//if (item_count > (actual_size >> 1))
		//{
		//	std::cout << "Hash array should be resized!" << "\n";
		//}

		uint32_t hash = index & mask;
		while (hasharray[hash] != -1)
		{
			hash++;
			if (hash >= actual_size) {
				hash = 0;
			}
		}
		item_count++;
		hasharray[hash] = index;
	}

	bool get(uint32_t index)
	{
		uint32_t hash = index & mask;
		while (hasharray[hash] != -1)
		{
			if (hasharray[hash] == index)
			{
				return true;
			}
			hash++;
			if (hash >= actual_size) {
				hash = 0;
			}
		}
		return false;
	}
};