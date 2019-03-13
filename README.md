# SAM

SAM is an extensible p-hacking simulator. It provides different interfaces and APIs for designing and experiment different statistical experiment and study the effects of various *p*-hacking method, a.k.a *questionable research practices*, on a defined experiment. For instance, one could design a two-by-two factorial experiment and study the effect of optional stopping on achieving significant results.

While there are a few conventional and well-known p-hacking methods are provided out of the box, the flexibility of SAM allows the user to define any specific methods, e.g., modifying measurements, and apply it on the currently existing experiment.

In this vignette, I’ll describe the underlying design of SAM and some of its capabilities alongside some examples. 

- [Introduction](doc/Introduction.md)
- [SAM's Main Components](doc/Components.md)
- [Configuration File Specifications](doc/ConfigurationFileSpecifications.md)
- [Components Interaction and Execution Flow](doc/ExecutionFlow.md)
- [Hacking Strategies](doc/HackingStrategies.md)
- [Decision Strategies](doc/DecisionStrategies.md)
- [SAM's API](doc/API.md)

If you like to install SAM before getting into more details, please follow the description below.

## Dependencies

- [Boost](https://github.com/docopt/docopt.cpp), General purpose C++ library 
- [docopt](https://github.com/docopt/docopt.cpp), CLI library 
- [Armadillo](http://arma.sourceforge.net/), C++ library for linear algebra & scientific computing
- [nlohmanh/json](https://github.com/nlohmann/json), JSON for Modern C++

On macOS, you can install all the dependencies using brew package manager by running, `brew tap nlohmann/json` and then `brew install boost docopt armadillo nlohmann_json`.

## Build

After successfully installing the dependencies, you can use CMAKE to create SAM’s executable using following commands:

	cd SAMpp
	mkdir build
	cd build
	cmake ..
	make

If everything goes right, you’ll have a file named `SAMpp` in your build directory which you’ll be able to use to run your simulation with it.

### Testing

SAM's unit test cases can be run using `make test`. Tests are designed to check the integrity of the code and API during the extension process, e.g., implementing new hacking strategy.

## A Sample Run

After successfully building SAMpp, you’ll be able to run your first simulation. Executing `./SAMpp` in your build folder will start a simulation based on the parameters located in `SAMpp/input/sample-input.json`. 

	$ ./SAMpp
	Initializing the simulation...
	
	Simulation output is saved in ../outputs/sample_simulation.csv
	



## Simulation Setup

In order to setup a simulation, SAM needs to know the specific representation of each of its components. All the necessary parameters can be listed in a JSON file and provided to SAM via the `--config` parameters of the CLI, e.g. `./SAM --config=simulation_parameters.json`




### QRP Methods

While SAM provides an interface to implement your own QRP algorithms, it will provide a few methods out of the box.

Read more on ...


## Outputs

