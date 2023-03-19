compiler = gcc
flags = -Wall -std=c99 -Wpedantic -Wextra -Werror=format-security -g -lm -O3

source_code = ./src/*.c

# Compiles everything
all: src/main.c
	@$(compiler) $(flags) -o ./bin/main $(source_code)

# Checks code complexity with lizard
lint: src/main.c
	@lizard -T parameter_count=9 -T token_count=500 -T length=150 -T cyclomatic_complexity=15 $(source_code)

# Runs main program
run:
	@./bin/main	

# Runs main against test 1
t1:
	@./bin/main < ./tests/T01/input.txt > ./tests/T01/my_result.txt
	@diff ./tests/T01/output.txt ./tests/T01/my_result.txt

# Runs main against test 2
t2:
	@./bin/main < ./tests/T02/input.txt > ./tests/T02/my_result.txt
	@diff ./tests/T02/output.txt ./tests/T02/my_result.txt

# Runs main against test 3
t3:
	@./bin/main < ./tests/T03/input.txt > ./tests/T03/my_result.txt
	@diff ./tests/T03/output.txt ./tests/T03/my_result.txt

# Runs main against test 4
t4:
	@./bin/main < ./tests/T04/input.txt > ./tests/T04/my_result.txt
	@diff ./tests/T04/output.txt ./tests/T04/my_result.txt

# Runs main against test 5
t5:
	@./bin/main < ./tests/T05/input.txt > ./tests/T05/my_result.txt
	@diff ./tests/T05/output.txt ./tests/T05/my_result.txt

# Runs main against test 6
t6:
	@./bin/main < ./tests/T06/input.txt > ./tests/T06/my_result.txt
	@diff ./tests/T06/output.txt ./tests/T06/my_result.txt

# Runs valgrind instance
valgrind:
	@docker run --platform linux/amd64 -tiv "$(PWD)/.:/valgrind" karek/valgrind:latest

# Cleans binaries
clean:
	@rm -rf ./bin/*