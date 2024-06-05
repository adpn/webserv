CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g
NAME = tmp

SRCS = 	main.cpp \
		Config.cpp \
		Server.cpp \
		Location.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re