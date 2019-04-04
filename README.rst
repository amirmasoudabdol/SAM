   **SAM is under development and testing!**

--------------

SAM is a C++ program, and soon a library for simulating and studying the
effects of different p-hacking method, questionable research practices
or other researcher’s degrees of freedom on the statistical inference
and meta-analysis metrics.

**Please visit the**\ `documentation <index.md>`__\ **page to learn more
about our design decisions and the available methods.**

If you like to install SAM before getting into more details, please
follow the instruction below.

Dependencies
------------

SAM uses a few well known C++ libraries to perform certain tasks.

-  `Boost <https://github.com/docopt/docopt.cpp>`__, General purpose C++
   library
-  `docopt <https://github.com/docopt/docopt.cpp>`__, CLI library
-  `Armadillo <http://arma.sourceforge.net/>`__, C++ library for linear
   algebra & scientific computing
-  `nlohmanh/json <https://github.com/nlohmann/json>`__, JSON for Modern
   C++
-  `CMake <https://cmake.org>`__, Automatic build system

On **macOS**, you can install all the dependencies using brew package
manager by running, ``brew tap nlohmann/json`` and then
``brew install boost docopt armadillo nlohmann_json cmake``.

On **Ubuntu**, you can use ``apt`` to install the listed packages by
running:

::

   sudo apt-get update
   sudo apt-get install liblapack-dev
   sudo apt-get install libblas-dev
   sudo apt-get install libboost-dev
   sudo apt-get install libarmadillo-dev

For ``nlohmann/json`` and ``docopt``, please check their documentation.
You probably must make and install them manually.

If you are using `conda <https://anaconda.org>`__ package manager you
can install all packages via ``conda`` as follow

::

   conda install -c anaconda boost
   conda install -c conda-forge armadillo
   conda install -c conda-forge nlohmann_json

You will still need to manually install ``docopt`` since it’s not
included in ``conda-forge`` repository.

Build Process
-------------

Once you have successfully installed all the dependencies, you can use
CMake to create SAM’s executable using following commands:

::

   git clone https://amirmasoudabdol.github.com/SAMpp
   cd SAMpp
   mkdir build
   cd build
   cmake ..
   make

If everything goes right, you’ll have a file named ``SAMpp`` in your
build directory which you’ll be able to use to run your simulation with
it.

Testing (Optional)
~~~~~~~~~~~~~~~~~~

You can run SAM‘s unit test cases using ``make test``. I design a few
tests to check the integrity of the code and API during the extension
process, e.g., implementing new hacking strategy. We will add more tests
as we improve our API.

A Sample Run
------------

After successfully building SAMpp, you’ll be able to run your first
simulation. Executing ``./SAMpp`` in your build folder will start a
simulation based on the parameters in ``SAMpp/input/sample-input.json``.

::

   $ ./SAMpp
   Initializing the simulation...

   Simulation output is saved in ../outputs/sample_simulation.csv

`More examples… <doc/examples.md>`__
