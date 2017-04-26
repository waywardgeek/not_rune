#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
struct _struct_xyIdent;
typedef struct _struct_xyIdent *xyIdent;
struct _struct_xyToken;
typedef struct _struct_xyToken *xyToken;
#else
typedef uint32 xyIdent;
typedef uint32 xyToken;
#endif

// Returns the token representing the type of the passed token.
typedef xyToken (*xyBindfunc)(xyIdent currentScope, xyToken token);
