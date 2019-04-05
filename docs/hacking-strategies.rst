.. _chap-hacking-strategies:

Hacking Strategies
==================

SAM can execute a complicated set of hacking strategies on a given
``Experiment``. A list of hacking strategies will be applied one-by-one
and in the given order. ``Researcher`` can observe their influences on
all outcome variables and decides when to stop, and what to report, see
:ref:`here design-hacking-strategy` and
:ref:`here flow-perform-research`.

The code below shows how you can define a chain of hacking strategies.
The ``--p-hacking-methods`` is a *list of list of JSON objects*, see
:ref:`crash course on JSON config-file-crash-course-on-json`. You
can define several groups of hacking strategies, each listing different
hacking strategies with different orders and parameters. Since JSON
arrays’ are ordered objects, this setup is suitable for controlling the
order in which each group, and method in each group will be applied on
the ``Experiment``. The example below defines 3 groups of hacking
strategies with a different number of methods in each.

.. code:: json

   {
     "--is-hacker": true,
     "--p-hacking-methods": [
       [ 
          {
              "name": "H1"
          }
         ,
          {
              "name": "H2"
          }
       ],
       [
          {
              "name": "H2"
          }
       ],
       [
          {
              "name": "H3"
          },
          {
              "name": "H4"
          },
          {
              "name": "H1"
          }
       ]
     ]
   }

A JSON object defines the specifications of each hacking strategy. The
code below defines *H1* hacking with three parameters. During the
:ref:`initialization phase flow-initialization`, SAM reads
the specification and prepare the hacking strategy accordingly.

.. code:: json

   {
     "name": "H1",
     "param-1": 1,
     "param-2": "yellow",
     "param-3": "at all cost"
   }

The rest of this section lists all hacking strategies implemented in
SAM. If you are interested in developing your own methods, head to
:ref:`chap-extension` section.

.. _hacking-strategies-optional-stopping:

Optional Stopping
-----------------

Optional stopping is the practice of adding new observations to
``Experiment``\’s data after initial data collection. The incentive
behind applying this method is often to achieve significant results. A
researcher can perform this method in many way. Here we have tried to
provide a set of parameters that can collectively mimic as many
scenarios as possible. To add optional stopping to your list of hacking,
you must replace one of the hacking methods, :math:`h_i` with the code
block below.

.. code:: json

   {
     "name": "Optional Stopping",
     "num": 3,
     "attempts": 3,
     "max attempts": 10,
     "level": "dv"
   }

The optional stopping algorithm is implemented based on the fact that
often a researcher perform multiple attempts to achieve significance.
Here, ``attempts`` defines the number of attempts and ``num`` specifies
the number of items — to be added — in each attempt.

+-----------------------+-----------------------+-----------------------+
| **Parameters**        | **Value**             | **Description**       |
+=======================+=======================+=======================+
| ``num``               | *n*, ``int``          | Number of             |
|                       |                       | observations to be    |
|                       |                       | added on each         |
|                       |                       | attempt.              |
+-----------------------+-----------------------+-----------------------+
| ``attempts``          | *t*, ``int``          | Number of attempts    |
|                       |                       | before stopping the   |
|                       |                       | process.              |
+-----------------------+-----------------------+-----------------------+
| ``max attempts``      | m, ``int``            | Maximum number of     |
|                       |                       | attempts              |
+-----------------------+-----------------------+-----------------------+
| ``level``             | “item”                | Adding new value to   |
|                       |                       | items in the          |
|                       |                       | underlying SEM.       |
|                       |                       | \ **Note:** Only      |
|                       |                       | applicable in Latent  |
|                       |                       | Model.                |
+-----------------------+-----------------------+-----------------------+
|                       | “dvs”                 | Adding new values to  |
|                       |                       | dependent variables.  |
+-----------------------+-----------------------+-----------------------+

You can control the intensity of optional stopping by alternating the
parameters. For instance, you can implement an *extreme* optional
stopping by setting ``num = 1`` and using large values for ``attempts``
and ``max_attempts``.

.. note::

    As discussed in the :ref:`data-strategies`
    section, optional stopping utilizes the ``DataStrategy`` for
    generating new datapoint.

.. _hacking-strategies-outliers-removal:

Outliers Removal
----------------

Removing outliers is another popular researcher’s degrees of freedom.
While use of outliers removal is not prohibited, most researcher are not
aware of its consequences if applies poorly.

Outliers removal method can be implemented in several different ways as
well. In the simplest case, a researcher will decide to remove a
datapoint from a dataset if the value is further than a distance from
the sample mean. One common method is to compare the distance of a value
to different multiplier of :math:`\sigma`. This type of outlier removal
can be deployed by defining the following parameters.

.. code:: json

   {
     "name": "SD Outlier Removal",
     "mode": "Recursive Attempts",  
     "level": "dv",   
     "num": 2,
     "n_attempts": 3,
     "max_attempts": 10,
     "min_observations": 20,
     "multipliers": [3, 2, 1]
   }

The main body of outliers removal algorithm is implemented similar to
the optional stopping, where the researcher will add ``num`` items in
``n_attempts`` before stopping the process, or achieving significant
results. Here though, you can also specify a list of ``multipliers`` to
indicate the distance of an item to the :math:`\sigma`. The algorithm
performs *t* attempts to remove *n* outliers from a dataset based on
given multipliers, :math:`\sigma_i`. The algorithm will advance if there
is no item left to be removed at :math:`i < n` attempts, or after *n*
attempts.

Table below describes all the available parameters and their valid
values.

+-----------------------+-----------------------+-----------------------+
| **Parameters**        | **Value**             | **Details**           |
+=======================+=======================+=======================+
| ``num``               | *n*, ``int``          | Number of items to be |
|                       |                       | removed at each       |
|                       |                       | attempt               |
+-----------------------+-----------------------+-----------------------+
| ``n_attempts``        | *t*, ``int``          | Number of attempts to |
|                       |                       | remove outliers for   |
|                       |                       | each multiplier       |
+-----------------------+-----------------------+-----------------------+
| ``max_attempts``      | ``int``               | Maximum number of     |
|                       |                       | iterations before     |
|                       |                       | stopping the process. |
+-----------------------+-----------------------+-----------------------+
| ``min_observations``  | ``int``               | The minimum number of |
|                       |                       | observations.         |
|                       |                       | Outliers removal      |
|                       |                       | stops removing values |
|                       |                       | when a group reaches  |
|                       |                       | ``min_observation``.  |
+-----------------------+-----------------------+-----------------------+
| ``multipliers``       | ``array``             | A list of multipliers |
|                       |                       | to be used.           |
+-----------------------+-----------------------+-----------------------+
| ``level``             | “dv”                  | Removing outliers at  |
|                       |                       | dependent variable    |
|                       |                       | level.                |
+-----------------------+-----------------------+-----------------------+
|                       | “item”                | Removing outliers at  |
|                       |                       | the item level, only  |
|                       |                       | applicable under      |
|                       |                       | Latent Model          |
|                       |                       | configuration.        |
+-----------------------+-----------------------+-----------------------+
| ``order``             | max first, random     |                       |
+-----------------------+-----------------------+-----------------------+

You can achieve different variants of outliers removal method by
modifying its parameters. For instance, setting ``num = 1`` and choosing
large values for ``n_attempts`` will remove the values one-by-one from
Experiment until it reaches the significance. You can control this
process by specifying a list of ``multipliers``.

.. _hacking-strategies-group-pooling:

Group Pooling
-------------

Group pooling is the act of pooling data from two or more groups into
one *new* group and compare the newly formed treatment group with the
control group. Group pooling can be applied by adding the following JSON
object to the list of hacking methods.

.. code:: json

   {
     "name": "Group Pooling",
     "num": 2
   }

Group pooling algorithm can pool different number of groups, ``num``,
together to form a new group. In its current setup, the Researcher
traverse through every permutation of length ``num`` and create a new
group. When, he collected all the combinations, he will then ask his
DecisionStrategy for ``verdict`` and consequently checks the
significance of every new group.

============== ========= ====================================================
**Parameters** **Value** **Details**
============== ========= ====================================================
``num``        ``int``   Indicates the number of groups to be pooled together
============== ========= ====================================================
