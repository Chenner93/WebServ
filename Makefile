  ####################
 #	NAME && FLAG	#
####################

NAME := webserv
CC := c++
# CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -MMD -g3
CXXFLAGS := -std=c++98 -MMD -g3

  ############
 #	COLOR	#
############

Black=\033[0;30m
Red=\033[0;31m
Green=\033[0;32m
Yellow=\033[0;33m
Blue=\033[0;34m
Purple=\033[0;35m
Cyan=\033[0;36m
White=\033[0;37m

  ################
 #	DIRECTORIES	#
################

SRC_DIR := srcs
OBJ_DIR := objs
INC_DIR := includes

  ################
 #	SRCS FILES	#
################

SRCS := \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/Server.cpp \
	$(SRC_DIR)/Client.cpp \


  ################################
 #	OBJS FILES && DEPENDANCIES	#
################################
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

  ################
 #	INCLUDES	#
################

INCLUDES := -I$(INC_DIR)

  ############
 #	RULES	#
############

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CXXFLAGS) $(INCLUDES) -o $@ $^
	@echo "$(Green)$(NAME) CREATED$(White)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	@$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
	@echo "$(Cyan)Compiling $<...$(White)"

clean:
	@echo "$(Red)Cleaning object files...$(White)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)
	@echo "$(Red)$(NAME) DELETED$(White)"

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
