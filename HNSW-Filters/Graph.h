#pragma once
#include<vector>
#include"Node.h"
#include"Layer.h"

using namespace std;

class KNodes
{
public:
	vector<Node> nodes;
	int K;

	KNodes()
	{
		K = 1000000000;
	}

	KNodes(int K)
	{
		this->K = K;
	}

	void InsertNode(Node node)
	{
		if (nodes.size() < K)
		{
			nodes.push_back(node);

			bool changed = true;

			while (changed)
			{
				changed = false;

				for (int i = 0; i < nodes.size() - 1; i++)
				{
					if (nodes[i].distance > nodes[i + 1].distance)
					{
						changed = true;
						Node tmpNode = nodes[i + 1];
						nodes[i + 1] = nodes[i];
						nodes[i] = tmpNode;
					}
				}
			}
		}
		else if (nodes[K - 1].distance > node.distance)
		{
			nodes[K - 1] = node;

			bool changed = true;

			while (changed)
			{
				changed = false;

				for (int i = 0; i < nodes.size() - 1; i++)
				{
					if (nodes[i].distance > nodes[i + 1].distance)
					{
						changed = true;
						Node tmpNode = nodes[i + 1];
						nodes[i + 1] = nodes[i];
						nodes[i] = tmpNode;
					}
				}
			}
		}
	}

	Node GetFirstNode()
	{
		return nodes[0];
	}

	Node GetLastNode()
	{
		return nodes[nodes.size() - 1];
	}

	void RemoveFirstNode()
	{
		nodes.erase(nodes.begin());
	}

	int Size()
	{
		return nodes.size();
	}

	Node* GetNodeForInsert()
	{
		return &nodes[0];
	}

	void PrintNodes()
	{
		printf("%d nearest points: \n", K);

		for (auto& n : nodes)
		{
			printf("\t");
			for (auto& v : n.values)
				printf("%d ", v);
			printf("\tdist: %f\n",n.distance);
		}

		printf("\n");
	}
};

class Graph
{
public:

	Node* entryPoint = nullptr;
	Node* candidate = nullptr;

	void InsertNode(Node& newNode)
	{
		/*printf("Adding: ");
		for (auto& v : newNode.values)
			printf("%d ", v);
		printf("\n");*/

		if (entryPoint == nullptr)
		{
			entryPoint = &newNode;
		}
		else if (entryPoint->neighbours.size() == 0)
		{
			entryPoint->InsertNode(newNode);
		}
		else
		{	
			Node* nearestNode = entryPoint;
			KNodes candidateNodes = KNodes();
			vector<Node*> visitedNodes;

			entryPoint->SetDistance(newNode);
			visitedNodes.push_back(entryPoint);
			candidateNodes.InsertNode(*entryPoint);

			while (candidateNodes.Size() > 0)
			{
				if (candidateNodes.GetFirstNode().distance > nearestNode->distance)
					break;

				vector<Node*> nbs = candidateNodes.GetFirstNode().neighbours;
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

					if(!nodeVisited)
					{

						visitedNodes.push_back(n);
						n->SetDistance(newNode);

						/*
						for (auto& v : n->values)
							printf("%d ", v);
						printf("\t %f\n", n->distance);
						*/
						if (n->distance < nearestNode->distance)
						{
							//printf("added\t%f < %f\n", n->distance, nearestNode->distance);

							nearestNode = n;
							candidateNodes.InsertNode(*n);
						}
					}
				}

				//printf("f ");

				//printf("x ");

				//candidateNodes.RemoveFirstNode();
			}
			//printf("d\n");
			nearestNode->InsertNode(newNode);

			
			/*for (auto& v : nearestNode->values)
				printf("%d ", v);
			printf("\t %f\n", nearestNode->distance);
			printf("DONE\n");*/
		}
	}

	KNodes GetKClosestNodes(Node queryNode, int K)
	{
		
		KNodes nearestNodes = KNodes(K);
		KNodes candidateNodes = KNodes();
		vector<Node*> visitedNodes;

		entryPoint->SetDistance(queryNode);
		
		candidateNodes.InsertNode(*entryPoint);
		nearestNodes.InsertNode(*entryPoint);
		visitedNodes.push_back(entryPoint);

		while (candidateNodes.Size() > 0)
		{
			if (candidateNodes.GetFirstNode().distance > nearestNodes.GetLastNode().distance)
				break;

			vector<Node*> nbs = candidateNodes.GetFirstNode().neighbours;
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
					n->SetDistance(queryNode);

					//for (auto& v : n->values)
					//	printf("%d ", v);
					//printf("\t %f\n", n->distance);

					if (n->distance < nearestNodes.GetLastNode().distance)
					{
						//printf("added\t%f < %f\n", n->distance, nearestNodes.GetLastNode().distance);

						nearestNodes.InsertNode(*n);
						candidateNodes.InsertNode(*n);
					}
				}
			}

			//printf("x ");
		}

		//printf("y\n");

		return nearestNodes;
	}

	




	void IsNodeInGraph(Node& baseNode, Node& newNode)
	{
		if (baseNode.GetDistance(newNode) <= 1)
		{
			printf("YEEEEEEES\n");
		}
		else
		{
			for (auto& n : baseNode.neighbours)
			{
				IsNodeInGraph(*n, newNode);
			}
		}
	}

	int cP = 0;
	int cL = 0;

	void GetAllNodes(Node& baseNode)
	{
		if (baseNode.neighbours.size() > 0)
		{
			printf("Parent: (%d)", baseNode.neighbours.size());
			for (auto v : baseNode.values)
			{
				printf("%d ", v);
			}
			printf("\n");

			cP++;
		}
		else
			cL++;

		/*
		for (auto& n : baseNode.neighbours)
		{
			printf("Child: ");
			for (auto& v : n->values)
			{
				printf("%d ", v);
			}
			printf("\n");
		}*/

		for (auto& n : baseNode.neighbours)
		{
			GetAllNodes(*n);
		}
	}

	void SearchInGraph(Node& baseNode, double distance, Node& newNode)
	{
		Node* newCandidate = nullptr;

		for (auto& n : baseNode.neighbours)
		{
			double newDist = n->GetDistance(newNode);

			if (newDist < distance)
			{
				newCandidate = n;
				distance = newDist;
			}
		}

		if (newCandidate != nullptr)
		{
			//printf("%d ", baseNode.neighbours.size());
			SearchInGraph(*newCandidate, distance, newNode);
		}
		else
		{
			printf("YEEES\n");
			candidate = &baseNode;
		}
	}

	Node GetClosestNode(Node node)
	{
		candidate = nullptr;

		SearchInGraph(*entryPoint, entryPoint->GetDistance(node), node);

		return *candidate;
	}


	vector<Node*> allNodes;

	void BFInsertNode(Node& node)
	{
		allNodes.push_back(&node);
	}
	
	Node BFGetClosestNode(Node node)
	{
		Node* clN = nullptr;

		for (auto& n : allNodes)
		{
			n->SetDistance(node);

			printf("%f\n", n->distance);

			if (clN == nullptr || n->distance < clN->distance)
			{
				clN = n;
			}
		}
		
		return *clN;

	}

};

