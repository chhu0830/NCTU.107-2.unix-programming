CC					?= gcc
LD					?= ld
ASM64				?= yasm

CFLAGS				+= -std=gnu99 -Wall -Wextra -pedantic -MMD -MF $@.d -fno-stack-protector -fPIC -nostdlib
LDFLAGS				+= -shared
ASMFLAGS			+= -f elf64

STUDENT_ID			:= 0756020
EXECUTABLE			:= libmini.so

OUT					?= .build
SRCS				:= $(wildcard */*.c */*.asm)
OBJS				:= $(addprefix $(OUT)/, $(SRCS:=.o))
DEPS				:= $(OBJS:.o=.o.d)

containing 			= $(foreach v,$(2),$(if $(findstring $(1),$(v)),$(v),))


all: $(EXECUTABLE)

$(EXECUTABLE): $(call containing,libmini,$(OBJS))
	$(LD) $^ -o $@ $(LDFLAGS)

$(OUT)/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	$(ASM64) $^ -o $@ $(ASMFLAGS)

$(OUT)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $< -o $@ -c $(CFLAGS)

mini_%: $(OUT)/src/start.asm.o $(OUT)/testcase/%.c.o
	$(LD) -m elf_x86_64 --dynamic-linker /lib64/ld-linux-x86-64.so.2 $^ -o $@ -L. -L.. -lmini
	LD_LIBRARY_PATH=. ./$@

zip:
	@rm -rf $(STUDENT_ID) $(STUDENT_ID).zip
	@mkdir $(STUDENT_ID)
	@git ls-files | xargs -i cp --parents {} $(STUDENT_ID)
	zip -r $(STUDENT_ID).zip $(STUDENT_ID)
	@rm -rf $(STUDENT_ID)

clean:
	rm -rf $(OUT) $(STUDENT_ID) $(EXECUTABLE)

-include $(DEPS) 
