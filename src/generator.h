//
// Created by Vladimir Ivanov on 2019-07-15.
//

#ifndef KTN_CPP_GENERATOR_H
#define KTN_CPP_GENERATOR_H

#include <iosfwd>

#include <vector>
#include <memory>

class TypeBase;
class Function;
class Class;


namespace generator {

/// @return false if skipped (not supported), true otherwise
bool genCDeclaration(std::ostream &os, const Function& f, bool doTypeStubs = false);

bool genCxxDefinition(std::ostream &os, const Function& f);
std::ostream& genCxxDefinition(std::ostream &os, const Class& c);

using Types = std::vector<std::unique_ptr<TypeBase>>;
void genWrappers(std::ostream& out_decl, std::ostream& out_def, Types::const_iterator begin, Types::const_iterator end);

bool genWrapper(std::ostream& str_decl, std::ostream& str_def, const Function& f);

}

/*
class WrapperDecl {
	std::ostringstream os;
	WrapperDecl& operator<<(const Function& f) {
		return f.genDecl(os);
	}
};
*/

#endif //KTN_CPP_GENERATOR_H
