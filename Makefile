DEPS = Makefile

CC=gcc

# Use these for the normal release
#CFLAGS=-std=c11 -Wall -O3 -march=native
#LIBS=-lddutil

# Use these for debugging
CFLAGS=-Wall -Wno-unused-function -g -std=c11 -D_POSIX_C_SOURCE -DDD_DEBUG -Iinclude
LIBS=-lddutil-dbg

SOURCE= \
core/analyze.c \
core/codatabase.c \
core/core.c \
core/coutil.c \
database/action.c \
database/debugging.c \
database/list.c \
database/map.c \
database/mtoken.c \
database/parser.c \
database/state.c \
database/string.c \
database/token.c \
database/xydatabase.c \
main/main.c \
parse/lexer.c \
parse/parse.c \
parse/utf8.c \
parsegen/itemset.c \
parsegen/parsegen.c \
parsegen/xpdatabase.c \
parsegen/ruleparse.c \
parsegen/rulelex.c

OBJS=$(patsubst %.c,obj/%.o,$(SOURCE))
GENFILES=database/xydatabase.c include/xydatabase.h parsegen/xpdatabase.c parsegen/xpdatabase.h parsegen/ruleparse.c parsegen/ruleparse.h parsegen/rulelex.c core/codatabase.c core/codatabase.h

all: obj rune

-include $(OBJS:.o=.d)

rune: $(GENFILES) $(DEPS) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o rune $(LIBS)

database/xydatabase.c: include/xydatabase.h

include/xydatabase.h: database/Database.dd
	cd database; datadraw -h ../include/xydatabase.h Database.dd

parsegen/xpdatabase.c: parsegen/xpdatabase.h

parsegen/xpdatabase.h: parsegen/Parsegen.dd
	cd parsegen; datadraw -I ../database Parsegen.dd

parsegen/ruleparse.c: parsegen/rules.y
	cd parsegen; bison -d -b xp -p xp -o ruleparse.c rules.y

parsegen/rulelex.c: parsegen/rules.l parsegen/ruleparse.h
	cd parsegen; flex -P xp -o rulelex.c rules.l

core/codatabase.c: core/codatabase.h

core/codatabase.h: core/Core.dd
	cd core; datadraw -I ../database Core.dd

clean:
	rm -f rune $(GENFILES)
	rm -rf obj

obj:
	mkdir -p obj/database obj/main obj/parsegen obj/parse obj/core

obj/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) -MM $(CFLAGS) $< | sed 's|^.*:|$@:|' > $(patsubst %.o,%.d,$@)
