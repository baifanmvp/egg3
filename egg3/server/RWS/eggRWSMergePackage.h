#ifndef EGGRWSMERGEPACKAGE_H_
#define EGGRWSMERGEPACKAGE_H_

#include "eggRWSCommon.h"
#include <egg3/Egg3.h>

HEGGNETPACKAGE eggRWSMergePackage_query(HEGGRWSFIFOQUEUE hPackages);

HEGGNETPACKAGE eggRWSMergePackage_query_iter(HEGGRWSFIFOQUEUE hPackages);

HEGGNETPACKAGE eggRWSMergePackage_get_docTotalCnt(HEGGRWSFIFOQUEUE hPackages);

#endif
