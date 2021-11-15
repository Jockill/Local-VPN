FLAGS = -g -Wall -Wextra

vpath %.c src/
vpath %.h head/

main: source destination medium

prof: source destination medium_prof

source: source.c obj/utils.o obj/fifo.o
	@echo "Construction de source..."
	@gcc $^ $(FLAGS) -o "source"
	@echo "Source construite!"

destination: destination.c obj/utils.o obj/fifo.o
	@echo "Construction de destination..."
	gcc $^ $(FLAGS) -o "destination"
	@echo "Destination construite!"

%.o: %.c %.h
	@mkdir -p obj/
	@gcc $(FLAGS) -c $< -o ./obj/$@


medium:
	@if [ -x ./getMedium.sh ]; then \
		./getMedium.sh; \
	else \
		echo "Vous n'avez pas les permissions pour executer getMedium.sh"; \
	fi

medium_prof:
	@if [ -x ./getMedium.sh ]; then \
		./getMedium.sh "PROF"; \
	else \
		echo "Vous n'avez pas les permissions pour executer getMedium.sh"; \
	fi

clean:
	rm -rf doc/* obj/* destination "source" medium.py *.o
