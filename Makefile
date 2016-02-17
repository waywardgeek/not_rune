DEPS = Makefile

CC=gcc

# Use these for the normal release
#CFLAGS=-std=c11 -Wall -O3 -march=native
#LIBS=-lddutil

# Use these for debugging
CFLAGS=-Wall -Wno-unused-function -g -std=c11 -D_POSIX_C_SOURCE -DDD_DEBUG
LIBS=-lddutil-dbg

SOURCE= \
itemset.c \
parse.c \
rulelex.c \
ruleparse.c \
rune.c \
string.c \
xydatabase.c

OBJS=$(patsubst %.c,obj/%.o,$(SOURCE))

all: obj rune

-include $(OBJS:.o=.d)

rune: xydatabase.c ruleparse.c rulelex.c $(DEPS) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o rune $(LIBS)

xydatabase.c: xydatabase.h

xydatabase.h: Rune.dd
	datadraw Rune.dd

ruleparse.c: rules.y
	bison -d -b xy -p xy -o ruleparse.c rules.y

rulelex.c: rules.l ruleparse.h
	flex -P xy -o rulelex.c rules.l

clean:
	rm -rf obj rune xydatabase.c xydatabase.h ruleparse.c ruleparse.h rulelex.c

obj:
	mkdir obj

obj/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) -MM $(CFLAGS) $< | sed 's|^.*:|$@:|' > $(patsubst %.o,%.d,$@)
