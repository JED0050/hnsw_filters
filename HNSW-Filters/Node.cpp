#pragma once

#include "Node.h"

Neighbours::Neighbours(int lID)
{
	layerID = lID;
}

void Neighbours::Insert(uint newNode)
{
	neighbours.push_back(newNode);
}

uint Neighbours::Size()
{
	return neighbours.size();
}

vector<uint> Neighbours::GetNeighbours()
{
	return neighbours;
}


Node::~Node()
{
	for (int i = 0; i < lNaighbours.size(); i++)
	{
		delete lNaighbours[i];
		lNaighbours[i] = nullptr;
	}

	//lNaighbours.clear();
}

void Node::Insert(uint node, int layerID)
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

void Node::SetNeighbours(Neighbours* nbrs)
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

Neighbours* Node::GetNeighboursAtLayer(int layerID)
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

vector<uint> Node::GetNeighboursVectorAtLayer(int layerID)
{
	for (auto& nbr : lNaighbours)
	{
		if (nbr->layerID == layerID)
		{
			return nbr->neighbours;
		}
	}

	vector<uint> emptyVector;
	return emptyVector;
}

void Node::InsertValue(float v, int index)
{
	values[index] = v;
}

float Node::GetDistance(vector<float> node)
{
	float distance = 0;

	for (uint i = 0; i < vectorSize; i++)
	{
		float x = node[i];
		float y = values[i];
		float z = x - y;

		distance += (z * z);
	}

	return distance;
}


NodeDist::NodeDist(uint id)
{
	ID = id;
}

NodeDist::NodeDist(uint id, float dist)
{
	ID = id;
	distance = dist;
}

void NodeDist::SetDistance(vector<float> n1, vector<float> n2)
{
	distance = 0;

	for (uint i = 0; i < Node::vectorSize; i++)
	{
		float x = n1[i];
		float y = n2[i];
		float z = x - y;

		distance += (z * z);
	}
}
