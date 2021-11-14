FLAGS = -g -Wall -Wextra

vpath %.h head/
vpath %.c src/
vpath %.o obj/

main: source destination medium
	@mv *.o -t ./obj
	@#Désolé on a pas trouvé mieux, vpath est facetieux

prof: source destination medium_prof
	@mv *.o -t ./obj

source: source.c utils.o fifo.o
	@echo "Construction de source..."
	@gcc $^ $(FLAGS) -o "source"
	@echo "Source construite!"

destination: destination.c utils.o fifo.o
	@echo "Construction de destination..."
	gcc $^ $(FLAGS) -o "destination"
	@echo "Destination construite!"

multi: multi.c utils.o fifo.o
	gcc $^ $(FLAGS) -o "multi"

obj/%.o: %.c %.h
	@mkdir -p obj/
	@gcc $(FLAGS) -c $< -o $@


medium:
	@if [ -x ./getMedium.sh ]; then \
		./getMedium.sh; \
	else \
		echo "Vous n'avez pas les permissions pour executer getMedium.sh"; \
	fi

medium_prof:
	@if [ -x ./getMedium.sh "PROF" ]; then \
		./getMedium.sh; \
	else \
		echo "Vous n'avez pas les permissions pour executer getMedium.sh"; \
	fi

clean:
	rm -rf doc/* obj/* destination "source" medium.py *.o
