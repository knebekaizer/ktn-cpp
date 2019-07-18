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


namespace ktn {

class WrapperGenerator {
public:
	WrapperGenerator(std::ostream& out_decl, std::ostream& out_def)
		: decl_(out_decl), impl_(out_def) {}

/// @return false if skipped (not supported), true otherwise
	bool genCDeclaration(const Function& f, bool toDeclStream = true);

	bool genCxxDefinition(const Function& f);
	std::ostream& genCxxDefinition(const Class& c);

	using Types = std::vector<std::unique_ptr<TypeBase>>;
	void genWrappers(Types::const_iterator begin, Types::const_iterator end);

	bool genWrapper(const Function& f);

	std::ostream& decl_;
	std::ostream& impl_;
};

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
