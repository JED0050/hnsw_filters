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
	vector<int> neighbours;

	Neighbours(int lID)
	{
		layerID = lID;
	}

	void Insert(int newNode)
	{
		neighbours.push_back(newNode);
	}

	int Size()
	{
		return neighbours.size();
	}

	vector<int> GetNeighbours()
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

	vector<int> GetNeighboursVectorAtLayer(int layerID)
	{
		for (auto& nbr : lNaighbours)
		{
			if (nbr->layerID == layerID)
			{
				return nbr->neighbours;
			}
		}

		vector<int> emptyVector;
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

		/*
		int nodeLastIndex = nodes.size() - 1;


		bool changed = true;

		while (changed)
		{
			changed = false;

			for (int i = 0; i < nodeLastIndex; i++)
			{
				if (hnswNodes[nodes[i]]->distance > hnswNodes[nodes[i + 1]]->distance)
				{
					changed = true;
					int tmpNode = nodes[i + 1];
					nodes[i + 1] = nodes[i];
					nodes[i] = tmpNode;
				}
			}
		}
		*/

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
		Sort();
		return nodes;
	}
};

class SortedNodesTuple
{
public:
	vector<tuple<unsigned int, float>> nodes;
	int K;

	tuple<unsigned int, float> minNode;
	tuple<unsigned int, float> maxNode;

	unsigned int maxIndex;

	SortedNodesTuple()
	{
		K = -1;
	}

	SortedNodesTuple(int K)
	{
		this->K = K;
	}

	void InsertNode(tuple<unsigned int, float> newNode)
	{
		if (nodes.empty())
		{
			nodes.push_back(newNode);

			minNode = newNode;
			maxNode = newNode;

			maxIndex = 0;
		}
		else if (K == -1 || nodes.size() < K)
		{
			nodes.push_back(newNode);

			if (get<1>(newNode) < get<1>(minNode))
			{
				minNode = newNode;
			}

			if (get<1>(newNode) > get<1>(maxNode))
			{
				maxNode = newNode;

				maxIndex = nodes.size() - 1;
			}

		}
		else if (get<1>(maxNode) > get<1>(newNode))
		{
			//nodes.erase(std::remove(nodes.begin(), nodes.end(), maxNode), nodes.end());
			//nodes.push_back(newNode);
			nodes[maxIndex] = newNode;

			//Sort();

			if (get<1>(newNode) < get<1>(minNode))
			{
				minNode = newNode;
			}

			for (int i = 0; i < nodes.size(); i++)
			{
				if (get<1>(nodes[i]) > get<1>(maxNode))
				{
					maxNode = nodes[i];
					maxIndex = i;
				}
			}
		}
	}

	int GetFirstNode()
	{
		//Sort();

		return get<0>(minNode);
	}

	int GetLastNode()
	{
		//Sort();

		return get<0>(maxNode);
	}

	void RemoveFirstNode()
	{
		if (nodes.size() <= 1)
		{
			nodes.clear();

			//minNode = nullptr;
			//maxNode = nullptr;

			return;
		}

		nodes.erase(nodes.begin());

		minNode = nodes[0];
		maxNode = nodes[0];

		for (int i = 1; i < nodes.size(); i++)
		{
			if (get<1>(nodes[i]) < get<1>(minNode))
			{
				minNode = nodes[i];
			}

			if (get<1>(nodes[i]) > get<1>(maxNode))
			{
				maxNode = nodes[i];
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
		sort(nodes.begin(), nodes.end(), TupleSortNearest());

		minNode = nodes[0];
		maxNode = nodes[Size() - 1];
	}

	vector<tuple<unsigned int, float>> GetKNearestNodes()
	{
		return nodes;
	}

	vector<tuple<unsigned int, float>> GetKNearestNodesSorted()
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
		Sort();
		//nodes.erase(nodes.begin());

		if (nodes.size() <= 1)
		{
			nodes.clear();
			return;
		}

		nodes.erase(nodes.begin());

		Sort();
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
		/*
		int nodeLastIndex = nodes.size() - 1;

		bool changed = true;

		while (changed)
		{
			changed = false;

			for (int i = 0; i < nodeLastIndex; i++)
			{
				if (hnswNodes[nodes[i]]->distance > hnswNodes[nodes[i + 1]]->distance)
				{
					changed = true;
					int tmpNode = nodes[i + 1];
					nodes[i + 1] = nodes[i];
					nodes[i] = tmpNode;
				}
			}
		}*/
	}

	vector<int> GetKNearestNodes()
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