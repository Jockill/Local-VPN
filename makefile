FLAGS = -g -Wall -Wextra

vpath %.c src/
vpath %.h head/

main: source destination medium

source: source.c utils.o fifo.o
	@echo "Construction de source..."
	@gcc src/source.c obj/utils.o obj/fifo.o $(FLAGS) -o "source"
	@echo "Source construite!"

destination: destination.c utils.o fifo.o
	@echo "Construction de destination..."
	@gcc src/destination.c obj/utils.o obj/fifo.o $(FLAGS) -o "destination"
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


clean:
	rm -rf doc/* obj/* destination "source" medium.py *.o
