FLAGS = -g -Wall -Wextra

ifndef VERBOSE
.SILENT:
endif

vpath %.h head/
vpath %.c src/
vpath %.o obj/

main: source serveur

source: source.c utils.o
	gcc $^ $(FLAGS) -o "source"

serveur: serveur.c utils.o
	gcc $^ $(FLAGS) -o "serveur"

utils.o: utils.c utils.h
	gcc $(FLAGS) -c $< -o $@


medium:
	if [ -x ./getMedium.sh ]; then \
		./getMedium.sh; \
	else \
		echo "Vous n'avez pas les permissions pour executer getMedium.sh"; \
	fi
