#include "serializer.util.hpp"

#include <algorithm>
#include <regex>

using namespace reflang;
using namespace std;

string serializer::getNameWithoutColons(string name)
{
	replace(name.begin(), name.end(), ':', '_');
	return name;
}

string serializer::replaceAll(
		const string &text,
		initializer_list<FromToPair> pairs)
{
	string result = text;
	for (const auto& pair : pairs)
	{
		regex replace(pair.first);
		result = regex_replace(result, replace, pair.second);
	}
	return result;
}
