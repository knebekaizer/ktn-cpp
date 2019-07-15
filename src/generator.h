//
// Created by Vladimir Ivanov on 2019-07-15.
//

#ifndef KTN_CPP_GENERATOR_H
#define KTN_CPP_GENERATOR_H

#include <iosfwd>

#include <vector>
#include <memory>

namespace reflang {
class TypeBase;
class Function;
class Class;
}


namespace generator {


std::ostream& genCxxDefinition(std::ostream &os, const reflang::Function& f);
std::ostream& genCxxDefinition(std::ostream &os, const reflang::Class& c);

void genCxxDefinition(std::ostream& os, const std::vector<std::unique_ptr<reflang::TypeBase>>& types);

}

/*
class WrapperDecl {
	std::ostringstream os;
	WrapperDecl& operator<<(const reflang::Function& f) {
		return f.genDecl(os);
	}
};
*/

#endif //KTN_CPP_GENERATOR_H
