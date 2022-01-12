#include "ServiceCheck.h"
#include "GlobalData.h"

bool ServiceCheck::serviceOnline() { return GlobalData::getServiceOnlineStatus(); }
