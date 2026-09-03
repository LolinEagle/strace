NAME		=	ft_strace
SRC			=	main.c \
				print.c \
				read_string.c \
				syscalls_32.c \
				syscalls_64.c \
				tracer.c
CC			=	gcc
RM			=	rm -rf
FLAGS		=	-Wall -Werror -Wextra
INC			=	-Iinc
SRC_PATH	=	./src/
OBJ_PATH	=	./obj/
OBJ_DIRS	=	$(OBJ_PATH)
OBJ			=	$(addprefix $(OBJ_PATH),$(SRC:.c=.o))
ERROR		:=	EMPTY_LINE_FUNCTION \
				GLOBAL_VAR_DETECTED \
				INVALID_HEADER \
				TOO_MANY_LINES \
				WRONG_SCOPE_COMMENT
IGNORE		:=	$(addprefix -e,$(ERROR))

all:$(NAME)

clean:
	$(RM) $(OBJ_PATH) .vscode

fclean:clean
	$(RM) $(NAME)

re:fclean
	$(MAKE) all

norm:
	@norminette | grep -v $(IGNORE) || true

test:re
	@clear && bash test_strace.sh

$(OBJ_PATH)%.o:$(SRC_PATH)%.c | $(OBJ_DIRS)
	$(CC) $(FLAGS) $(INC) -c $< -o $@

$(OBJ_DIRS):
	mkdir -p $(OBJ_DIRS)

$(NAME):$(OBJ)
	$(CC) $(FLAGS) $(INC) $(OBJ) -o $(NAME)

.PHONY:all clean fclean re norm test
