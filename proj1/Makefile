CC          ?= gcc
CFLAGS      += -std=gnu99 -Wall -Wextra -MMD -MF $@.d

STUDENT_ID  := 0756020
EXECUTABLE  := hw1

OUT         ?= .build
SRCS        := $(wildcard src/*.c)
OBJS        := $(addprefix $(OUT)/, $(SRCS:.c=.o))
DEPS        := $(OBJS:.o=.o.d)

$(EXECUTABLE): $(OBJS) 
	$(CC) $(OBJS) -o $(EXECUTABLE)

$(OUT)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

run: $(EXECUTABLE)
	./$(EXECUTABLE)

zip:
	@rm -rf $(STUDENT_ID) $(STUDENT_ID).zip ; mkdir $(STUDENT_ID)
	@git ls-files | xargs -i cp --parents {} $(STUDENT_ID)
	@zip -r $(STUDENT_ID).zip $(STUDENT_ID) ; rm -rf $(STUDENT_ID)

clean:
	rm -rf ./test/test_* $(OUT) $(STUDENT_ID) $(STUDENT_ID).zip $(EXECUTABLE)

-include $(DEPS) 
