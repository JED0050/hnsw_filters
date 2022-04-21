#pragma once

#include <vector>
#include <algorithm>
#include <tuple>

#define uint unsigned int

using namespace std;

class Node;
class Neighbours
{
public:
	vector<uint> neighbours;
	int layerID;

	Neighbours(int lID);

	void Insert(uint newNode);
	uint Size();
	vector<uint> GetNeighbours();

};


class Node
{
public:
	static uint vectorSize;
	//float values[128];
	vector<float> values;
	vector<Neighbours*> lNaighbours;

	~Node();

	void Insert(uint node, int layerID);
	void InsertValue(float v, int index);
	void SetNeighbours(Neighbours* nbrs);

	Neighbours* GetNeighboursAtLayer(int layerID);
	vector<uint> GetNeighboursVectorAtLayer(int layerID);

	float GetDistance(vector<float> node);
};

class NodeDist
{
public:
	uint ID;
	float distance;

	NodeDist(uint id);
	NodeDist(uint id, float dist);

	void SetDistance(vector<float> n1, vector<float> n2);
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

	uint32_t item_count = -1;

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