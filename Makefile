points = 4
tests = 1

chmod:
	chmod -R +xr .

compile:
	./scripts/compile.sh ${points}

run:
	make chmod
	make clear
	./scripts/run.sh ${points} ${tests}

clear:
	./scripts/clear.sh 6 ${tests}
