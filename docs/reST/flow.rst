.. _chap-flow:

Flow
====

SAM simulates the scientific process through several subprocesses as
depicted in Figure 1. Each rounded rectangle describes a
process, often performs by one of the components or through
collaboration between different components. This chapter describes each
subprocess in more details.

.. figure:: figures/main-routine.png
   :align: center
   
   Overall execution flowchart of SAM

.. _flow-initialization:

Initialization
--------------

During the initialization phase, SAMs loads a configuration file and
initializes different compartments based on configuration parameters. You can read
more about the parameters `here <configuration-file.rst>`__.


.. figure:: figures/initialization.png
   :align: center

After reading user parameters, SAM proceeds with initializing each of
its components. Initialization mainly prepares the internal specifications of each compartment. At the end of initialization stage, ``Researcher`` is aware of all parameters and is ready to conduct the — already defined — ``Experiment``.

.. _flow-prepare-research:

Prepareing the Research
-----------------------

The preparation of the research is the first step of the simulation.
This step resembles the process of  collecting the data for the study. 
At this stage, the ``Researcher`` uses the information provided in the 
``ExperimentSetup`` to produce/collect data points for each group/condition/dv using the ``DataStrategy`` module.


.. As we discussed in the :doc:`design` chapter, ``ExperimentSetup`` is a
.. read-only object.

..  except the case that a user prefers to randomize one of
.. its parameters in each different runs of SAM, for instance, randomizing
.. the number of observations to study its effect on a particular metric.
.. This can be done by setting ``nobs`` parameters to “random”. Here, SAM
.. first randomizes the ``ExperimentSetup`` before making it a read-only
.. object.

.. The next step is allocating memory for the data, and analysis through
.. the simulation. And finally,

.. ``Researcher`` uses  ``Experiment`` uses the data provided by the ``ExperimentSetup`` to produce the data for the hypothesis. As we discussed, this is being done using the ``DataStrategy`` subroutine. After producing the data, data for each group/condition/dv are stored ``measurements`` variable.

.. figure:: figures/prepare-research.png
   :align: center

   Steps involving preparing the research

.. _flow-perform-research:

Performing the Research
-----------------------

.. *Performing the research mainly resembles the data processing process.*

At this stage, the ``Researcher`` uses ``TestStrategy`` (of
the ``Experiment``) to run the test and populates relevant parameters, e.g.,
``statistic, pvalue, sig, side``. The next step is to check whether or not the ``Researcher`` is satisfied with test results. This is being done by passing the ``Experiment`` to the ``DecisionStrategy``. ``Researcher`` relies on deicsion strategy's verdict to decide whether to proceed with the current Submission or to continue applying one or more hacking strategies on the experiment before submitting the study for the review.

As mentioned, Researcher can have access to a list of hacking methods. For
each hacking strategy, **h**, a copy of experiment is
passed to the hacking strategy. Based on the type of hacking strategy,
``Researcher`` reaches to the ``DecisionStrategy`` on whether it should continue
or stop the process of altering the result. For instance, with optional stopping, the researcher — after each phase of addition — can decide whether the outcome is satisfactory or not.

.. , *intermediate verdict.*

.. figure:: figures/perform-research.png
   :align: center

   Steps involving performing the research

.. _flow-publish-research:

Publishing the Research
-----------------------

Process of publishing the research resembles the preparation of final
mamuscript and sending it to the Journal for review.

After applying the last hack, ``Researcher`` continues with the
preparation of its **final** Submission. This is being done through the ``DecisionStrategy`` where the researcher asks for the *final verdict*. Based on the type of ``DecisionStrategy``, s/he can look back at
different versions of ``Experiment`` or ``Submission`` and pick the most
satisfying record.

After asking for final verdict, ``Researcher`` is ready to submit its finding.  Journal will judge the submitted ``Submission``\’s based on
the criteria defined in its ``SelectionStrategy`` routine. If accepted, ``Journal`` adds the submission to its publication lists for further analysis or
adjustment of its ``SelectionStrategy``, if necessary.

.. figure:: figures/publish-research.png
   :align: center

   Steps involving publishing a research

