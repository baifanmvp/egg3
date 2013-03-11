#ifndef EGG3_H_
#define EGG3_H_


#ifdef __cplusplus
extern "C" {
#endif 

#include "./EggDef.h"

#include "./eggDirectory.h"
#include "./eggHttp.h"
#include "./eggPath.h"
#include "./eggCluster.h"
#include "./eggAnalyzer.h"
#include "./eggDocument.h"
#include "./eggField.h"
#include "./eggIndexReader.h"
#include "./eggIndexWriter.h"
#include "./eggQuery.h"
#include "./eggScoreDoc.h"
#include "./eggDid.h"
#include "./eggTopCollector.h"
#include "./eggIndexSearcher.h"
#include "./index/eggFieldView.h"
#include "./index/eggIndexView.h"
#include "index/eggFieldWeight.h"
#include "./net/eggNetServer.h"
#include "./conf/eggConfig.h"
#include "./storage/eggIdTable.h"
#include "./net/eggSpanUnit.h"
#include "./net/eggClientInfoServ.h"
#include "./uti/eggThreadPool.h"
#include "./log/eggPrtLog.h"

#ifdef __cplusplus
}
#endif 

#endif //EGG_H_
