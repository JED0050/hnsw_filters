#pragma once

#include <iostream>
#include <vector>
#include <tuple>

#define uint unsigned int

using namespace std;

class DimFilter
{
public:
	vector<float> eqNumbers;
	vector<tuple<float, float>> intervals;
	bool emptyFilter;
	uint index;

	DimFilter(uint ind);

	void AddEqNumber(float val);
	void AddInterval(float start, float end);

	bool IsDimValid(float val);
};

static class DimFilterHelper
{
public:
	static vector<DimFilter> GetNumOfFilters(int n);
	static bool IsVectorValid(vector<DimFilter> filters, vector<float> vec);

	static vector<DimFilter> GenerateFilter(uint dims, float perc, int min, int max);
	static vector<DimFilter> GenerateFilterRandom(uint dims, int min, int max);
	static vector<DimFilter> GenerateFilterTotalRandom(uint dims, int min, int max);

	static vector<DimFilter> LoadFilterFromString(string filterString);
	static string GetFilterString(vector<DimFilter> filter);
};



