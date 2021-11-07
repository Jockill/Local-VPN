FLAGS = -g -Wall -Wextra

ifndef VERBOSE
.SILENT:
endif

vpath %.h head/
vpath %.c src/
vpath %.o obj/

main: source destination

source: source.c utils.o
	gcc $^ $(FLAGS) -o "source"

destination: destination.c utils.o
	gcc $^ $(FLAGS) -o "destination"

obj/utils.o: utils.c utils.h
	mkdir -p obj/
	gcc $(FLAGS) -c $< -o $@


medium:
	if [ -x ./getMedium.sh ]; then \
		./getMedium.sh; \
	else \
		echo "Vous n'avez pas les permissions pour executer getMedium.sh"; \
	fi

clean:
	rm -rf doc/* obj/* destination "source" medium.py
