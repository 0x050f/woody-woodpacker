# COLORS #
#
_RED		=	\e[31m
_GREEN		=	\e[32m
_YELLOW		=	\e[33m
_END		=	\e[0m

# COMPILATION #

CC			=	gcc

CC_FLAGS	=	-Wall -Wextra -Werror

NASM		=	nasm

NASM_FLAGS	=	-f elf64

# DIRECTORIES #

DIR_HEADERS =	./includes/

DIR_SRCS	=	./srcs/

DIR_OBJS	=	./compiled_srcs/

DIR_OBJS_ASM	=	./compiled_srcs/

# FILES #

SRCS			=	woody_woodpacker.c \
					utils.c

SRCS_ASM		=	inject.s

# COMPILED_SOURCES #

OBJS 		=	$(SRCS:%.c=$(DIR_OBJS)%.o)

OBJS_ASM	=	$(SRCS_ASM:%.s=$(DIR_OBJS)%.o)

NAME 		=	woody_woodpacker

## RULES ##

all:			$(NAME)

debug:			CC_FLAGS += -g3 -fsanitize=address
debug:			all

# VARIABLES RULES #

$(NAME):		$(OBJS) $(OBJS_ASM)
				@printf "\033[2K\r$(_GREEN) All files compiled into '$(DIR_OBJS)'. $(_END)✅\n"
				@$(CC) $(CC_FLAGS) -I $(DIR_HEADERS) $(OBJS) $(OBJS_ASM) -o $(NAME)
				@printf "\033[2K\r$(_GREEN) Executable '$(NAME)' created. $(_END)✅\n"

# COMPILED_SOURCES RULES #

$(OBJS):		| $(DIR_OBJS)

$(OBJS_ASM):	| $(DIR_OBJS_ASM)

$(DIR_OBJS)%.o: $(DIR_SRCS)%.c
				@printf "\033[2K\r $(_YELLOW)Compiling $< $(_END)⌛ "
				@$(CC) $(CC_FLAGS) -I $(DIR_HEADERS) -c $< -o $@

$(DIR_OBJS_ASM)%.o: $(DIR_SRCS)%.s
				@printf "\033[2K\r $(_YELLOW)Compiling $< $(_END)⌛ "
				@$(NASM) $(NASM_FLAGS) -o $@ $<

$(DIR_OBJS):
				@mkdir -p $(DIR_OBJS)

# OBLIGATORY PART #

clean:
				@rm -rf $(DIR_OBJS)
				@printf "\033[2K\r$(_RED) '"$(DIR_OBJS)"' has been deleted. $(_END)🗑️\n"
				@rm -rf woody
				@printf "\033[2K\r$(_RED) 'woody' has been deleted. $(_END)🗑️\n"

fclean:			clean
				@rm -rf $(NAME)
				@printf "\033[2K\r$(_RED) '"$(NAME)"' has been deleted. $(_END)🗑️\n"

re:				fclean all

# PHONY #

.PHONY:			all debug clean fclean re
