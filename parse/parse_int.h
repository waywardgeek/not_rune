#include "database.h"
#include "utf8.h"
#include "parse.h"

// Parsing stuff
void paLexerStart(void);
void paLexerStop(void);
xyToken paLex(void);
void paPrintToken(xyToken token);
void paError(xyToken token, char *message, ...);

extern FILE *paFile;
extern xyParser paCurrentParser;
extern uint32 paLinenum;
