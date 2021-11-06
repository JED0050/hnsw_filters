#pragma once
#include <vector>

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
	vector<long> values;
	vector<Neighbours*> lNaighbours;
	double distance = -1;

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
		for (auto& nbr : lNaighbours)
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

	void InsertValue(long value)
	{
		values.push_back(value);
	}

	void SetDistance(Node& node)
	{
		distance = GetDistance(node);
	}

	double GetDistance(Node& node)
	{
		double distance = 0;

		for (int i = 0; i < node.values.size(); i++)
		{
			double x = node.values[i];
			double y = values[i];
			double z = x - y;

			distance += (z * z);
		}

		return distance;
		//return sqrt(distance);
	}
	

	
	vector<Node*> neighbours;

	void InsertNode(Node& node)
	{
		neighbours.push_back(&node);
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

	SortedNodes(vector<Node*> allNodes)
	{
		hnswNodes = allNodes;
		K = -1;
	}

	SortedNodes(vector<Node*> allNodes, int K)
	{
		hnswNodes = allNodes;
		this->K = K;
	}

	void InsertNode(int node)
	{
		if (K == -1 || nodes.size() < K)
		{
			nodes.push_back(node);

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
		return minV;
	}

	int GetLastNode()
	{
		return maxV;
	}

	void RemoveFirstNode()
	{
		//Sort();
		//nodes.erase(nodes.begin());

		if (nodes.size() <= 1)
		{
			nodes.clear();
			return;
		}

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

	void Sort()
	{
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
	}

	vector<int> GetKNearestNodes()
	{
		Sort();

		return nodes;
	}
};
