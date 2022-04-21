#pragma once

#include "Layer.h"

int Layer::maxLayer = 0;

Layer::Layer(int eP)
{
	entryPoint = eP;
	ID = maxLayer;
	maxLayer++;
	numberOfNodes = 0;
}
