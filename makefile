FLAGS = -g -Wall -Wextra


vpath %.h head/
vpath %.c src/
vpath %.o obj/

main: source destination medium

source: source.c utils.o
	echo "Construction de source..."
	@gcc $^ $(FLAGS) -o "source"
	echo "Source construite!"

destination: destination.c utils.o
	echo "Construction de destination..."
	gcc $^ $(FLAGS) -o "destination"
	echo "Destination construite!"

obj/utils.o: utils.c utils.h
	@mkdir -p obj/
	@gcc $(FLAGS) -c $< -o $@


medium:
	@if [ -x ./getMedium.sh ]; then \
		./getMedium.sh; \
	else \
		echo "Vous n'avez pas les permissions pour executer getMedium.sh"; \
	fi

clean:
	rm -rf doc/* obj/* destination "source" medium.py
