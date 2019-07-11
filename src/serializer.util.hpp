#ifndef REFLANG_SERIALIZER_UTIL_HPP
#define REFLANG_SERIALIZER_UTIL_HPP

#include <string>
#include <utility>

namespace reflang
{
	namespace serializer
	{
		std::string mangled(std::string name);
		std::string getNameWithoutColons(std::string name);

		using FromToPair = std::pair<std::string, std::string>;
		std::string replaceAll(
				const std::string &text,
				std::initializer_list<FromToPair> pairs);
	}
}

#endif //REFLANG_SERIALIZER_UTIL_HPP


