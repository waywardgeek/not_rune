#include "database.h"
#include "padatabase.h"
#include "utf8.h"
#include "parse.h"

// Parsing stuff
void paLexerStart(xyParser parser);
void paLexerStop(void);
paToken paLex(void);
void paPrintToken(paToken token);
void paError(paToken token, char *message, ...);

extern FILE *paFile;
extern uint32 paLineNum;
