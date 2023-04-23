points = 6
tests = 3

chmod:
	chmod -R +xr .

compile:
	./scripts/compile.sh ${points}

run:
	make chmod
	./scripts/run.sh ${points} ${tests}
