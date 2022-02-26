#pragma once
#include <vector>
#include <tuple>

using namespace std;

class DimFilter
{
public:
	vector<float> eqNumbers;
	vector<tuple<float, float>> intervals;
	bool emptyFilter;

	DimFilter()
	{
		emptyFilter = true;
	}

	void AddEqNumber(float val)
	{
		emptyFilter = false;
		eqNumbers.push_back(val);
	}

	void AddInterval(float start, float end)
	{
		emptyFilter = false;
		intervals.push_back(make_tuple(start, end));
	}

	bool IsDimValid(float val)
	{
		if (emptyFilter)
			return true;

		for (auto &i : intervals)
		{
			if (get<0>(i) <= val && val <= get<1>(i))
				return true;
		}

		for (auto& v : eqNumbers)
		{
			if (v == val)
				return true;
		}

		return false;
	}

};

static class DimFilterHelper
{
public:
	static vector<DimFilter> GetNumOfFilters(int n)
	{
		vector<DimFilter> newFilters;

		for (int i = 0; i < n; i++)
		{
			newFilters.push_back(DimFilter());
		}

		return newFilters;
	}

	static bool IsVectorValid(vector<DimFilter> filters, vector<float> vec)
	{

		for (int i = 0; i < vec.size(); i++)
		{
			if (!filters[i].IsDimValid(vec[i]))
			{
				return false;
			}
		}

		return true;
	}
};



