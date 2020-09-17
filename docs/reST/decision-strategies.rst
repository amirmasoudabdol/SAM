.. _chap-decision-strategies:

Decision Strategies
===================


.. pull-quote::
    
    *Decision Strategy executes researcher’s logic behind starting and stopping the questionable research practices.*

Decision strategy is one of the important components of SAM. It tries to
implement ``Researcher``\’s intuition and logic during different stages
of the research. 

.. While I have briefly covered the 
.. this section lists all the available methods and clarifies their implementation details.

The main responsibility of the ``DecisionStrategy`` is to inform the
``Researcher`` about the state of results, i.e., whether a significant
result exists, a hacking method should be applied or continued, are findings publishable, etc.

When
----

One important aspect of the decision-making process is *when* in which the
decision is being made. The ``Researcher`` can make his/her decision (1)
right after running the *first* test (2) during the hacking process (3)
after completing one hack, and (4) just before applying his study to a
``Journal``. Various decision strategies can behave differently on every
stage and therefore leading to different results.

The ``DecisionStrategy`` class distinguishes between these stages by utilizing a ``DecisionStage``
variable. ``DecisionStage`` can accepts 4 different values as follow:

1. ``Initial``
2. ``WhileHacking``
3. ``DoneHacking``
4. ``Final``
   
What
----

Another aspect of ``DecisionStrategy`` is *what* that the ``Researcher`` considers as a satisfactory result. 
One can think of this as *researcher’s goal*, especially when *p*-hacking methods are
involved. ``Researcher`` needs to know whether he is going to only
concern about the significance of the pre-registered outcome or he
*prefers* outcomes with minimal *p*-values, or other criteria.

SAM comes with a set of pre-defined parameters, ``DecisionPreference``, as
follows:

-  ``PreRegisteredOutcome``. Only reporting the pre-registered
   outcome
-  ``MinPvalue``. Selecting the outcome with minimum *p*-value.
-  ``MinSigPvalue``.
-  ``RandomSigPvalue``.
-  ``MaxSigPvalue``.
-  ``MaxSigEffect``.
-  ``MaxEffect``. Selecting the outcome with maximum effect size.
-  ``MinPvalueMaxEffect``. Selecting the *significant* outcome with
   maximum effect size.

In the case or ``PreRegisteredOutcome``, the Researcher always reports
the pre-registered result, regardless of significance.

.. In other 
.. therefore, s/he is attempting to achieve significant results on the
.. pre-registered outcome.

.. even if it applies some *p*-hacking methods;


How
---

Besides *When* and *What* of the decision, a researcher could carry a specific 
behavior in his/her deicison making process. The *How* mainly comes to play when 
the researcher is being confronted by many options to choose from. For instnace, 
in the case of a "*p*-hacker", the researcher has to have an strategy on how 
he is going to choose between various results from various *p*-hacking paths that s/he took.


.. _decision-strategies-honest:

Honest Decision Maker
~~~~~~~~~~~~~~~~~~~~~

A “honest researcher” always reports the pre-registered result and will
not apply any *p*-hacking methods on the Experiment.

.. note::
  
  This is the default *decision making* method.

.. _decision-strategies-impatient:

Impatient Decision Maker
~~~~~~~~~~~~~~~~~~~~~~~~

An “impatient researcher” stops his effort as soon as s/he finds a
significant result. This could be before, during or after applying
*p*-hacking methods. You can specify an *impatient decision maker* by
setting the ``decision_strategy`` value, as follow:

.. code:: json

   {
     "name": "ImpatientDecisionMaker",
     "preference": "MinPvalue"
   }

.. _decision-strategies-patient:

Patient Decision Maker
~~~~~~~~~~~~~~~~~~~~~~

A “patient researcher” behaves differently from the impatient
researcher. While he checks the progress of his effort, he does not make
a conclusion until he tries everything. He collects all significant
results and based on his preferences reports the best of them.

For instance, the specification below defines a researcher who — after applying
all hacking methods — reports the outcome with largest effect size,
``MaxEffect``.

.. code:: json

   {
     "name": "PatientDecisionMaker",
     "preference": "MaxEffect"
   }
