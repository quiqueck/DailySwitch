#include "NopSerial.h"

#ifdef DEBUG_LOG
#else
NopSerial NullSerial(0);
#endif