include Makefile.config

.PHONY: all obj install uninstall clean unit_test unit_test_dev valgrind fmt
.DELETE_ON_ERROR:

PREFIX          := /usr/local
INCDIR          := $(PREFIX)/include
LIBDIR          := $(PREFIX)/lib
SRCDIR          := src
DEPSDIR         := deps
TESTDIR         := t
EXAMPLEDIR      := examples
INCDIR          := include

DYNAMIC_TARGET  := $(LIBNAME).so
STATIC_TARGET   := $(LIBNAME).a
EXAMPLE_TARGET  := example
TEST_TARGET     := test

SRC             := $(wildcard $(SRCDIR)/*.c)
TESTS           := $(wildcard $(TESTDIR)/*.c)
DEPS            :=
TEST_DEPS       := $(wildcard $(DEPSDIR)/*/*.c)
OBJ             := $(addprefix obj/, $(notdir $(SRC:.c=.o)) $(notdir $(DEPS:.c=.o)))

INCLUDES        := -I$(INCDIR) -I$(DEPSDIR)
LIBS            :=
CFLAGS          := -Wall -Wextra -pedantic $(INCLUDES)

ifdef TAP_WANT_PTHREAD
	LIBS += -lpthread
endif

ifdef TAP_WANT_PCRE
	LIBS += -lpcre
endif

$(DYNAMIC_TARGET): CFLAGS += -shared -fPIC
$(DYNAMIC_TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

$(STATIC_TARGET): $(OBJ)
	$(AR) rcs $@ $^

obj/%.o: $(SRCDIR)/%.c $(INCDIR)/$(LIBNAME).h | obj
	$(CC) $< -c $(CFLAGS) -o $@

obj/%.o: $(DEPSDIR)/*/%.c | obj
	$(CC) $< -c $(CFLAGS) -o $@

$(EXAMPLE_TARGET): $(STATIC_TARGET)
	$(CC) $(CFLAGS) $(EXAMPLEDIR)/main.c $< $(LIBS) -o $@

all: $(DYNAMIC_TARGET) $(STATIC_TARGET)

obj:
	@mkdir -p obj

install: $(STATIC_TARGET)
	@mkdir -p ${LIBDIR} && cp -f ${STATIC_TARGET} ${LIBDIR}/$@
	@mkdir -p ${INCDIR} && cp -r $(INCDIR)/$(LIBNAME).h ${INCDIR}

uninstall:
	@rm -f ${LIBDIR}/$(STATIC_TARGET)
	@rm -f ${INCDIR}/libys.h

clean:
	@rm -f $(OBJ) $(STATIC_TARGET) $(DYNAMIC_TARGET) $(EXAMPLE_TARGET) $(TEST_TARGET)

unit_test: $(STATIC_TARGET)
	$(CC) -lpcre -lpthread -DTAP_WANT_PTHREAD=1 -DTAP_WANT_PCRE=1        \
	$(CFLAGS) $(TESTS) $(TEST_DEPS) $(STATIC_TARGET) -I$(SRCDIR) $(LIBS) \
	-o $(TEST_TARGET)
	./$(TEST_TARGET)
	$(MAKE) clean

unit_test_dev:
	ls $(SRCDIR)/*.{h,c} $(TESTDIR)/*.{h,c} | entr -s 'make -s unit_test'

valgrind: $(STATIC_TARGET)
	$(CC) $(CFLAGS) $(TESTS) $(TEST_DEPS) $< $(LIBS) -o $(TEST_TARGET)
	$(VALGRIND) --leak-check=full --track-origins=yes -s ./$(TEST_TARGET)
	@$(MAKE) clean

fmt:
	@$(FMT) -i $(wildcard $(SRCDIR)/*) $(wildcard $(TESTDIR)/*) $(wildcard $(INCDIR)/*) $(wildcard $(EXAMPLEDIR)/*)
