CC					?= gcc
CFLAGS				+= -std=gnu99 -Wall -Wextra -pedantic -MMD -MF $@.d -fPIC
LDFLAGS				+= -shared -ldl

STUDENT_ID			:= 0756020
EXECUTABLE			:= hw2.so

OUT					?= .build
SRCS				:= $(wildcard src/*.c)
OBJS				:= $(addprefix $(OUT)/, $(SRCS:.c=.o))
DEPS				:= $(OBJS:.o=.o.d)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OUT)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $< -o $@ -c $(CFLAGS)

test: $(EXECUTABLE)
	MONITOR_OUTPUT=fsmon.log LD_PRELOAD=./$(EXECUTABLE) head -n 1000 /etc/services > /dev/null
	LD_PRELOAD=./$(EXECUTABLE) ls -la

zip:
	@rm -rf $(STUDENT_ID) $(STUDENT_ID).zip ; mkdir $(STUDENT_ID)
	@git ls-files | xargs -i cp --parents {} $(STUDENT_ID)
	@zip -r $(STUDENT_ID).zip $(STUDENT_ID) ; rm -rf $(STUDENT_ID)

clean:
	rm -rf $(OUT) $(STUDENT_ID) $(STUDENT_ID).zip $(EXECUTABLE)

-include $(DEPS) 
