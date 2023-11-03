# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rchiewli <rchiewli@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/07/16 20:52:32 by psuanpro          #+#    #+#              #
#    Updated: 2023/11/03 23:20:09 by rchiewli         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= webserv

MAIN		= main.cpp
MAIN_DIR	= ./

CC			= g++
CPPFLAGS	= -g -Wall -Werror -Wextra -fsanitize=address


SRC_DIR		= ./src/

MODULE1		=	Conf.cpp

MODULE2		= Server.cpp \
				Response.cpp \
				Request.cpp \
 				HttpRequestHandle.cpp

MODULE3		= CgiHandler.cpp

MODULE4		= Console.cpp \
				Uti.cpp \
				StringMatrix.cpp \
				File.cpp \
				Detail.cpp


MODULE1_DIR	= ./src/parsing/
MODULE2_DIR = ./src/server/
MODULE3_DIR = ./src/cgihandler/
MODULE4_DIR = ./src/utils/


OBJ_DIR		= ./obj/

OBJS		= ${addprefix $(OBJ_DIR),$(MODULE1:.cpp=.o)} \
			  ${addprefix $(OBJ_DIR),$(MODULE2:.cpp=.o)} \
			  ${addprefix $(OBJ_DIR),$(MODULE3:.cpp=.o)} \
			  ${addprefix $(OBJ_DIR),$(MODULE4:.cpp=.o)} \
			  ${addprefix $(OBJ_DIR),$(MAIN:.cpp=.o)}

RM = rm -rf


BRED =\033[1;31m
BGRN =\033[1;32m
BYEL =\033[1;33m
RES = \033[0m
BLU = \033[0;34m

COMPILE = echo "$(BGRN)$(NAME) compiled....$(RES)"
CLEAN = echo "$(BYEL)$(NAME) clean....$(RES)"
FCLEAN = echo "$(BRED)$(NAME) fclean....$(RES)"


#parsing
${OBJ_DIR}%.o: ${MODULE1_DIR}%.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $^

#server
${OBJ_DIR}%.o: ${MODULE2_DIR}%.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $^

#cgihandler
${OBJ_DIR}%.o: ${MODULE3_DIR}%.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $^

#utils
${OBJ_DIR}%.o: ${MODULE4_DIR}%.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $^


all: ${OBJ_DIR} ${NAME}



${OBJ_DIR}:
	@mkdir -p $(OBJ_DIR)

${OBJ_DIR}%.o: ${MAIN_DIR}%.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $^

${NAME}: ${OBJS}
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME) $(LIB)

clean:
	@$(RM) $(OBJ_DIR)
	@$(RM) *.dSYM
	@$(CLEAN)

fclean: clean
	@$(RM) $(NAME) $(OBJ_DIR)
	@$(FCLEAN)


re: fclean all

.PHONY: all clean fclean re run
