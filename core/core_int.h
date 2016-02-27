#include "database.h"
#include "core.h"
#include "codatabase.h"

void coBindIdentifiers(xyToken token, xyIdent parentScope);

// Utility module
void coUtilStart(void);
void coUtilStop(void);
void coWrtemp(FILE *file, char *temp, ...);
char *coSwrtemp(char *temp, ...);
utSym coUpperSym(utSym sym);
void coError(xyToken token, char *message, ...);

// Globals
extern FILE *coFile;
