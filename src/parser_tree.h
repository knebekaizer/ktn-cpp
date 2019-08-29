//
// Created by Володя on 2019-08-21.
//

#ifndef KTN_CPP_PARSER_TREE_H
#define KTN_CPP_PARSER_TREE_H

#include <vector>

#include "utils.h"

void parseTypes(
		const std::vector<std::string>& files,
		int argc, char** argv,
		const Options& options = Options());


#endif //KTN_CPP_PARSER_TREE_H
