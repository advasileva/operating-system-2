points = 7
tests = 7

chmod:
	chmod -R +xr .

compile:
	./scripts/compile.sh ${points}

run:
	make chmod
	make clear
	./scripts/run.sh ${points} ${tests}

clear:
	./scripts/clear.sh 9 ${tests}
