#pragma once

#include "DimFilter.h"

vector<DimFilter> DimFilterHelper::GenerateFilter(uint dims, float perc, int min, int max)
{
	if (perc < 0)
		perc = 0;
	else if (perc > 1)
		perc = 1;

	uint filtDims = dims * perc;

	if (filtDims == 0 && perc > 0)
		filtDims = 1;

	vector<uint> genIdx;
	vector<DimFilter> filter;

	while (filter.size() < filtDims)
	{
		uint index = 0;

		bool found = false;

		while (!found)
		{
			index = rand() % dims;

			found = true;

			for (auto v : genIdx)
			{
				if (v == index)
				{
					found = false;
					break;
				}
			}
		}

		genIdx.push_back(index);
		DimFilter newDim = DimFilter(index);

		float val1 = rand() % ((max - min) * 100);
		val1 /= 100;
		val1 += min;

		float val2 = rand() % ((max - min) * 100);
		val2 /= 100;
		val2 += min;

		if (val1 > val2)
		{
			float tmpVal = val1;
			val1 = val2;
			val2 = tmpVal;
		}

		if (val1 < min)
			val1 = min;

		if (val2 > max)
			val2 = max;

		if ((val2 - val1) < 10)
			val2 += 10;

		if (val2 > max)
			val2 = max;

		if ((val2 - val1) < 10)
			val1 -= 10;

		if (val1 < min)
			val1 = min;

		newDim.AddInterval(val1, val2);

		filter.push_back(newDim);

		cout << "\tIdx: " << index << " Min: " << val1 << " Max: " << val2 << endl;
	}

	return filter;
}

vector<DimFilter> DimFilterHelper::GenerateFilterRandom(uint dims, int min, int max)
{
	uint tier = rand() % 50; //èím více atributù tím rozsáhlejší filtr
	tier /= 10; //0 - 4
	tier += 1;	//1 - 5

	float ftier = (float)tier / (float)10; //0.1-0.5

	vector<DimFilter> filter;

	int nMin = min;
	int nMax = max;
	
	int part = (max - min) / 20;

	nMin += part * (6 - tier);
	nMax -= part * (6 - tier);

	filter = GenerateFilter(dims, ftier, nMin, nMax);
	
	return filter;
}