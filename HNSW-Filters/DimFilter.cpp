#pragma once

#include <iostream>
#include "DimFilter.h"

using namespace std;

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

		if ((val2 - val1) < 10)
			val2 += 10;

		if (val1 < min)
			val1 = min;

		if (val2 > max)
			val2 = max;

		newDim.AddInterval(val1, val2);

		filter.push_back(newDim);

		cout << "Idx: " << index << " Min: " << val1 << " Max: " << val2 << endl;
		//printf("Idx: %.2f  Min: %g  Max: %lg\n", index, val1, val2);
	}

	return filter;
}