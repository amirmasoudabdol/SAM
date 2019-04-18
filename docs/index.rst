.. SAMpp documentation master file, created by
   sphinx-quickstart on Wed Apr  3 15:54:03 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

SAM
======

SAM is an extensible p-hacking simulator. It provides different
interfaces and APIs for designing — and experimenting with — different
statistical experiments under the influence of various *p*-hacking
methods or strategies as well as some *questionable research practices*.
For example, one could design a two-by-two factorial experiment and
study the effect of optional stopping on achieving significant results.
While number of conventional and well-known *p*-hacking methods are
provided out of the box, the flexibility of SAM allows the user to
define any specific methods, e.g., modifying measurements based on a
specific metric.

In this vignette, I’ll describe the underlying design principles of SAM
and some of its capabilities alongside some examples. In the
Introduction section, I’ll describe the problem that SAM is trying to
solve and how are we going to approach it. After setting the ground
rules, we can dive into different components of SAM and the process of
designing your own simulation.


.. toctree::
   :maxdepth: 2

   introduction
   design
   flow
   configuration-file
   data-strategies
   hacking-strategies
   decision-strategies


.. Indices and tables
.. ==================

.. * :ref:`genindex`
.. * :ref:`modindex`
.. * :ref:`search`
