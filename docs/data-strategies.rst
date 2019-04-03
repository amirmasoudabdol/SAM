Data Strategies
===============

``DataStrategy`` provides data for the ``Experiment``. A ``Researcher``
during the process of `preparing the
research <#ExecutionFlow.md#prepare-research>`__ reaches to the
``DataStrategy`` object and based on the underlying model and parameters
of the ``Experiment`` — stored in ``ExperimentSetup`` — populates its
``measurements`` variable. With this routine, we have tried to simulate
the *process of collecting data* in a scientific research.

Since ``DataStrategy`` is aware of all underlying model, parameters and
distribution, it can provide data at any points during the simulation.
One important implication of this is during the `optional
stopping <HackingStrategies.md#optional-stopping>`__ where
``Researcher`` needs to collect *new* data from the same population he
started from.

You can select the type of your model by setting ``--data-strategy``
parameters of the `config file <ConfigurationFileSpecficications.md>`__.
Two available options are *Linear Model* and *Latent Model*. Based on
your model of choice, SAM only uses fractions of parameters provided in
the configuration file. In this section, we discuss how SAM uses the
parameters to set up the model and how it generates data.

Linear Model
------------

SAM determines the total number of groups, :math:`n_g` (internal
variable), by multiplying the number of treatment conditions,
:math:`n_c`, by the number of dependent variables in each condition,
:math:`n_d`. After knowing the number of groups, each group is being
populated by :math:`n_o` observations from a multi-variate normal
distribution with the mean of :math:`\mu` and covariance matrix of
:math:`\sigma`, :math:`X \sim MN(\mu, \Sigma)`.

You can change the parameters accordingly to implement your own design.
For example, in order to set up an experiment with no covariance, you
may set the ``--covs`` to ``0``. This will prompt SAM to only use the
diagonal row of the covariance matrix.

================== =========================================
**Parameters**     **Value**
================== =========================================
``--n-conditions`` :math:`n_c`, ``int``
``--n-dep-vars``   :math:`n_d`, ``int``
``--n-obs``        :math:`n_o`, ``int``
``--means``        :math:`\mu`, ``double`` or ``array``
``--vars``         :math:`\sigma^2`, ``double`` or ``array``
``--covs``         :math:`\Sigma`, ``double``, ``matrix``
================== =========================================

Latent Model
------------

============== =============================================
**Parameters** **Value**
============== =============================================
``--n-items``  :math:`n_i`, ``int``, or ``array``
``--loadings`` :math:`\lambda`, ``double`` or ``array``
``--err-vars`` :math:`\epsilon_\mu`, ``double`` or ``array``
``--err-covs`` :math:`\epsilon_\sigma`, ``double``
============== =============================================
