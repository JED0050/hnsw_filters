#pragma once
#include <vector>
#include <algorithm>
#include <tuple>

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

	unsigned int Size()
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

	~Node()
	{
		for (auto n : lNaighbours)
		{
			delete n;
		}
		lNaighbours.clear();
	}

	void Insert(unsigned int node, int layerID)
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

	void InsertValue(float v, int index)
	{
		values[index] = v;
	}

	float GetDistance(vector<float> node)
	{
		float distance = 0;

		for (unsigned int i = 0; i < vectorSize; i++)
		{
			float x = node[i];
			float y = values[i];
			float z = x - y;

			distance += (z * z);
		}

		return distance;
	}

};

class NodeDist
{
public:
	unsigned int ID;
	float distance;

	NodeDist(unsigned int id)
	{
		ID = id;
	}

	NodeDist(unsigned int id, float dist)
	{
		ID = id;
		distance = dist;
	}

	void SetDistance(vector<float> n1, vector<float> n2)
	{
		distance = 0;

		for (unsigned int i = 0; i < Node::vectorSize; i++)
		{
			float x = n1[i];
			float y = n2[i];
			float z = x - y;

			distance += (z * z);
		}
	}
};

struct NodeDistanceSortNearest {
	bool operator()(const NodeDist& a, const NodeDist& b) const {
		return a.distance < b.distance;
	}
};

struct NodeDistanceSortFurthest {
	bool operator()(const NodeDist& a, const NodeDist& b) const {
		return a.distance > b.distance;
	}
};

//https://github.com/RadimBaca/HNSW/blob/master/src/hnsw.h
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