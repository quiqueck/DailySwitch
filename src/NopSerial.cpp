#include "NopSerial.h"

#if DEBUG_LOG
#else
NopSerial NullSerial(0);
#endif