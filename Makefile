# Name of the executable
NAME		=	webserv

# Compiler, compiler flags
CC			=	c++
FLAGS		=	-Wall -Wextra -Werror -std=c++98 -Iincludes
DFLAGS		=	-fsanitize=address -g

# Directories for sources files, object files, and the libft library
SRCS_DIR	= 	srcs
BUILD_DIR 	= 	build
#replace all smth of course
SMTH_DIR	=   $(SRCS_DIR)/smth 
SMTH2_DIR	=   $(SRCS_DIR)/smth2

# Define the source files
MAIN_FILE	=	main.c
SMTH_FILES	=
SMTH2_FILES	=

# Defining the paths of the sources files
SRC_MAIN	= 	$(addprefix $(SRCS_DIR)/,$(MAIN_FILE))
SRC_SMTH  	=	$(addprefix $(SMTH_DIR)/, $(SMTH_FILES))
SRC_SMTH2 	=	$(addprefix $(SMTH2_DIR)/, $(SMTH2_FILES))

# Deriving objects from .cpp files in the build directory
OBJS 		= 	$(patsubst $(SRCS_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_MAIN)) \
				$(patsubst $(SMTH_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_SMTH)) \
				$(patsubst $(SMTH2_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_SMTH2))

# Display toolbox
RED			=	\x1b[31m
YELLOW		=	\x1b[33m
GREEN		=	\x1b[32m
NO_COLOR	=	\x1b[0m
BOLD		= 	\x1b[1m
BOLD_OFF	=	\x1b[21m

# Rules to build the objects from the sources
$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SMTH_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SMTH2_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

# Rule to build the executable from the objects 
$(NAME): $(OBJS)
	@echo "$(YELLOW)Creating $(NAME)... $(NO_COLOR)"
	@$(CC) $(CFLAGS) -o $@ $^
	@echo "$(GREEN)$(BOLD)Enjoy!$(BOLD_OFF)$(NO_COLOR)"

# Phony target to build the executable
all: $(NAME)

# Phony target to clean the object files
clean:
	@echo "$(RED)Deleting objects...$(NO_COLOR)"
	@rm -rf $(BUILD_DIR)

# Phony target to remove the executable and build objects
fclean: clean
	@echo "$(RED)Deleting executable...$(NO_COLOR)"
	@rm -f $(NAME)

# Phony target to perform a full re-build
re: fclean all

# Rule to build with debug flags
debug: CFLAGS += $(DFLAGS)
debug: re

# Phony targets for make
.PHONY: all clean fclean re debug
