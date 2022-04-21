#pragma once

#include "Node.h"

using namespace std;

class Layer
{
public:
	static int maxLayer;
	int ID;
	unsigned int entryPoint;
	int numberOfNodes;

	Layer(int eP);
};

