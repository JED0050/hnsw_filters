#pragma once

#include "Node.h"

using namespace std;

class Layer
{
public:
	static int maxLayer;
	int ID;
	unsigned int entryPoint;
	int numberOfNodes = 0;

	Layer(int eP)
	{
		entryPoint = eP;
		ID = maxLayer;
		maxLayer++;
	}

};

