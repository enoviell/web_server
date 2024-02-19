NAME = web_server

MAKEFLAGS += --silent

SRC =	srcs/main.cpp \
		srcs/web_server/WebServer.cpp \
		srcs/web_server/Socket.cpp \
		srcs/web_server/Location.cpp \
		srcs/config_praser/AstGeneration.cpp \
		srcs/config_praser/ConfigParsing.cpp \
		srcs/HTTP_parser/HTTPParser.cpp \
		srcs/HTTP_responder/deleteResponder.cpp \
		srcs/HTTP_responder/httpResponder.cpp \
		srcs/HTTP_responder/postResponder.cpp \
		srcs/HTTP_responder/getResponder.cpp \

OBJ = ${SRC:.cpp=.o}

CC = c++

RM = rm -f

CFLAGS = -g -W -W -W -std=c++98

.o:.cpp
	${CC} ${CFLAGS} -c $< -o ${<:.cpp=.o}

${NAME}:  ${OBJ}
	${CC} $(CFLAGS) ${OBJ} -o $(NAME)
	@echo "\033[32mCompiled OK!\033[0m"

all: ${NAME}

clean:
		${RM} ${OBJ}
		@echo "\033[35mCleared everything!\033[0m"

fclean: clean
		${RM} ${NAME}

re: fclean all
