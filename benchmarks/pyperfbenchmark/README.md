This folder contains microbenchmarks from the Python Performance Benchmarks Suite.

To add a new microbenchmark, do the following:
1. Create a folder with the name of the bmk
2. Create a zip file with exactly the same name as the bmk containing the following files:
	a. __main__.py with main(params) function
	b. virtualenv/ with the required third-party packages
3. Update the deploy.sh script to include the new bmk in the list
