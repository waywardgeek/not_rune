DEPS = Makefile

CC=gcc

# Use these for the normal release
#CFLAGS=-std=c11 -Wall -O3 -march=native
#LIBS=-lddutil

# Use these for debugging
CFLAGS=-Wall -Wno-unused-function -g -std=c11 -D_POSIX_C_SOURCE -DDD_DEBUG -Iinclude
LIBS=-lddutil-dbg

SOURCE= \
main/main.c \
database/action.c \
database/debugging.c \
database/parser.c \
database/string.c \
database/mtoken.c \
database/value.c \
database/xydatabase.c \
parse/itemset.c \
parse/parse.c \
parse/xpdatabase.c \
parse/ruleparse.c \
parse/rulelex.c

OBJS=$(patsubst %.c,obj/%.o,$(SOURCE))
GENFILES=database/xydatabase.c include/xydatabase.h parse/xpdatabase.c parse/xpdatabase.h parse/ruleparse.c parse/ruleparse.h parse/rulelex.c

all: obj rune

-include $(OBJS:.o=.d)

rune: $(GENFILES) $(DEPS) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o rune $(LIBS)

database/xydatabase.c: include/xydatabase.h

include/xydatabase.h: database/Database.dd
	cd database; datadraw -h ../include/xydatabase.h Database.dd

parse/xpdatabase.c: parse/xpdatabase.h

parse/xpdatabase.h: parse/Parse.dd
	cd parse; datadraw -I ../database Parse.dd

parse/ruleparse.c: parse/rules.y
	cd parse; bison -d -b xp -p xp -o ruleparse.c rules.y

parse/rulelex.c: parse/rules.l parse/ruleparse.h
	cd parse; flex -P xp -o rulelex.c rules.l

clean:
	rm -f rune $(GENFILES)
	rm -rf obj

obj:
	mkdir -p obj/database obj/main obj/parse

obj/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) -MM $(CFLAGS) $< | sed 's|^.*:|$@:|' > $(patsubst %.o,%.d,$@)
