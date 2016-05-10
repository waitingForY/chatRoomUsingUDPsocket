#将clean 和all 设置为伪目标
.PHONY:clean all
CC=cc
#-Wall 是提示所有警告信息，-g是在可执行文件中加入调试信息
CFLAGS=-g
BIN=chatcli chatserv
all:$(BIN)
%.o:%.c
	#$<表示单个的目标,$@表示所有的目标
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)


