#ifndef REFLANG_SERIALIZER_FUNCTION_HPP
#define REFLANG_SERIALIZER_FUNCTION_HPP

#include <iostream>
#include <string>

#include "serializer.hpp"
#include "types.hpp"

namespace reflang
{
	namespace serializer
	{
		std::string GetFunctionSignature(const Function& f);
		void SerializeFunctionHeader(std::ostream& o, const Function& c);
		void SerializeFunctionSources(std::ostream& o, const Function& c);

		std::string mangling(std::string s);
		std::ostream& genDefinition(std::ostream& os, const Function& f);
	}
}

#endif //REFLANG_SERIALIZER_FUNCTION_HPP
