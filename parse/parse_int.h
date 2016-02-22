#include "database.h"
#include "padatabase.h"
#include "utf8.h"
#include "parse.h"

// Parsing stuff
void paLexerStart(xyParser parser);
void paLexerStop(void);
paToken paLex(bool ignoreNewlines);
void paPrintToken(paToken token);
void paError(paToken token, char *message, ...);

extern FILE *paFile;
extern uint32 paLineNum;
