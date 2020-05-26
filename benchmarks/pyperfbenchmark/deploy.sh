#Usage: set FAAS_ROOT as home directory for faas-profiler and run the script to deploy all pyperfbenchmarks
export WSK_CONFIG_FILE=~/openwhisk-devtools/docker-compose/.wskprops

PYPERF_DIR=$FAAS_ROOT/functions/pyperfbenchmark
BENCHMARKS=()

for ENTRY in "$PYPERF_DIR"/*
do
    if [[ $ENTRY == *"bm_"* ]] && [[ $ENTRY != *".zip"* ]] && [[ $ENTRY != *".py"* ]] && [[ $ENTRY != *"virtualenv"* ]]; then
	BENCHMARK=${ENTRY#"$PYPERF_DIR/"}
	BENCHMARKS=( "${BENCHMARKS[@]}" "$BENCHMARK" )
    fi
done

wsk -i action list > tmp_actions
FILE_NAME=tmp_actions

for BENCHMARK in "${BENCHMARKS[@]}"
do
	if grep -q $BENCHMARK "$FILE_NAME"; then
	    echo "$BENCHMARK action already invoked!"
	else
	    echo "$BENCHMARK"
            wsk -i action create $BENCHMARK --kind python:3 $PYPERF_DIR/$BENCHMARK/$BENCHMARK.zip --web true
	fi
done

rm $FILE_NAME
