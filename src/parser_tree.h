//
// Created by Володя on 2019-08-21.
//

#ifndef KTN_CPP_PARSER_TREE_H
#define KTN_CPP_PARSER_TREE_H

#include <vector>

#include "utils.h"

using Args = std::vector<std::string>;

void parseTypes(
		const std::vector<std::string>& files,
		const Args& args //, const Options& options = Options()
				);


#endif //KTN_CPP_PARSER_TREE_H
