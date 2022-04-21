#pragma once

#include "DimFilter.h"
#include <algorithm>
#include <string>
#include <iomanip>
#include <sstream>

DimFilter::DimFilter(uint ind)
{
	index = ind;
	emptyFilter = true;
}

void DimFilter::AddEqNumber(float val)
{
	emptyFilter = false;
	eqNumbers.push_back(val);
}

void DimFilter::AddInterval(float start, float end)
{
	emptyFilter = false;
	intervals.push_back(make_tuple(start, end));
}

bool DimFilter::IsDimValid(float val)
{
	if (emptyFilter)
		return true;

	for (auto& i : intervals)
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


vector<DimFilter> DimFilterHelper::GetNumOfFilters(int n)
{
	vector<DimFilter> newFilters;

	for (int i = 0; i < n; i++)
	{
		newFilters.push_back(DimFilter(i));
	}

	return newFilters;
}

bool DimFilterHelper::IsVectorValid(vector<DimFilter> filters, vector<float> vec)
{

	if (filters.size() == 0)
	{
		return true;
	}

	for (int i = 0; i < filters.size(); i++)
	{
		uint vecIdx = filters[i].index;

		if (!filters[i].IsDimValid(vec[vecIdx]))
		{
			return false;
		}
	}

	return true;
}

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

		//cout << "\tIdx: " << index << " Min: " << val1 << " Max: " << val2 << endl;
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

vector<DimFilter> DimFilterHelper::GenerateFilterTotalRandom(uint dims, int min, int max)
{
	uint actDims = (rand() % 4) + 1;//(rand() % dims) + 1;

	vector<uint> indexes;

	while (indexes.size() < actDims)
	{
		uint index = rand() % dims;

		if (!(find(indexes.begin(), indexes.end(), index) != indexes.end()))
		{
			indexes.push_back(index);
		}
	}

	sort(indexes.begin(), indexes.end());

	vector<DimFilter> filter;

	for (auto i : indexes)
	{
		DimFilter nF = DimFilter(i);

		/*float diff = min + (float)(rand() % (max - min) * 100) / 100;
		diff /= 100;
		float fMin = min + diff;
		float fMax = max - diff;*/

		//float fMin = min + (float)(rand() % (int)((max - min) * 100)) / 100;
		//float fMax = min + (float)(rand() % (int)((max - min) * 100)) / 100;

		float fMin = min + (rand() % (int)(max - min));
		float fMax = min + (rand() % (int)(max - min));

		if (rand() % 10000 >= 9000)
			fMin = 0;

		if (fMin < fMax)
			nF.AddInterval(fMin, fMax);
		else
			nF.AddInterval(fMax, fMin);

		filter.push_back(nF);
	}

	return filter;
}

vector<DimFilter> DimFilterHelper::LoadFilterFromString(string filterString)
{
	vector<DimFilter> filter;

	uint cursor = 0;

	while (cursor < filterString.length())
	{
		uint vectorIndex = stoi(filterString.substr(cursor, filterString.find(":", cursor)));

		DimFilter filterPart = DimFilter(vectorIndex);

		cursor = filterString.find(":", cursor) + 1;

		while (cursor < filterString.length() && filterString[cursor] != ';')
		{
			if (filterString[cursor] == '<')
			{
				cursor++;

				uint del = filterString.find(",", cursor);
				float val1 = stof(filterString.substr(cursor, del - cursor));

				cursor = del + 1;
				del = filterString.find(">", cursor);
				float val2 = stof(filterString.substr(cursor, del - cursor));

				filterPart.AddInterval(val1, val2);

				cursor = del;
			}
			else if (filterString[cursor] == '(')
			{
				cursor++;

				uint del = filterString.find(")", cursor);
				float val = stof(filterString.substr(cursor, del - cursor));
				filterPart.AddEqNumber(val);

				cursor = del;
			}

			cursor++;
		}
		cursor++;


		filter.push_back(filterPart);

	}

	return filter;
}

string DimFilterHelper::GetFilterString(vector<DimFilter> filter)
{
	//0:<50.55,250.00>(0.00);2:<10.00,20.00><30.00,100.30>;4:<0.00,100.00><150.00,200.00>;

	string filterString = "";

	for (auto f : filter)
	{
		filterString += to_string(f.index);
		filterString += ":";

		for (auto interval : f.intervals)
		{
			filterString += "<";

			stringstream stream;
			stream << std::fixed << std::setprecision(2) << get<0>(interval);
			string val = stream.str();
			filterString += val;

			filterString += ",";

			stream.str("");
			stream << std::fixed << std::setprecision(2) << get<1>(interval);
			val = stream.str();
			filterString += val;

			filterString += ">";
		}

		for (auto number : f.eqNumbers)
		{
			filterString += "(";

			stringstream stream;
			stream << std::fixed << std::setprecision(2) << number;
			filterString += stream.str();

			filterString += ")";
		}

		filterString += ";";
	}

	return filterString;
}
