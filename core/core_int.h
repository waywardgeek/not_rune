#include "database.h"
#include "core.h"
#include "codatabase.h"

bool coAnalyze(xyList prog);

// Utility module
void coUtilStart(void);
void coUtilStop(void);
void coWrtemp(FILE *file, char *temp, ...);
char *coSwrtemp(char *temp, ...);
utSym coUpperSym(utSym sym);
