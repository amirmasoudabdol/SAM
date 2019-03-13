<div style="text-align: right"><i>Design</i></div><br>

In the previous section, I have tried to extract main components and entities involving in different stages of a research, e.g., *Experiment Setup, Experiment, Researcher, Submission*, and *Journal*. In the abstraction, each component  can be seen as a loosely independent entity while the whole system, i.e., *scientific research*, can be described by a set of behaviors, tasks and interactions.

One of our main design goals with SAM was to achieve a level of flexibility where we could modify different aspects of the research process relatively easily. In order to achieve this,  we have tried to decouple the system to smaller but conceptually meaningful scientific routines and objects. Figure 1. shows all the components of SAM and their dependencies alongside their interactions with each other. 

This section will clarify the design principles behind each component, how they work and interact with each other in order to simulate the entire process.

> Figure 1.

## SAM’s Main Components

SAM consists of 3 main components, *Experiment, Researcher* and *Journal*. Each component tries to summarize one of the mentioned subprocesses or entities of a scientific research as described in the [Introduction](Introduction.md). The list below briefly introduces each component and its task and role.

- The [**Experiment**](#experiment) consists of several subroutines and objects dealing with different aspect of a research, e.g., setup, data, test. 
	- [*ExperimentSetup*](#experiment-setup) holds the necessary information and parameters of how the experiment should be defined. Parameters of the ExperimentSetup cannot be modified during the simulation and only can be set at the start of the experiment. In fact, it resembles the *pre-registration*.
	- [*Data Strategy*](#data-strategy) is a routine used to generate the necessary data based on the parameters specified in *ExperimentSetup*.
	- [*Test Strategy*](#test-strategy) is a statistical method of choice in the Experiment Setup for testing the result of the experiment.
- The **Researcher** object imitates the behaviors of a researcher including his possible questionable research practices. The researcher will define the *ExperimentSetup*, generate/collect the data, run the statistical test, decides whether to preform any QRPs, prepare the *Submission* record, and finally submit it to his *Journal* of choice.
- The **Journal** is a container of *Submission*s, i.e., published studies. Journal can decide — though a *SelectionStrategy* — whether a submission is going to be accepted or not. The Journal keeps track of its publications and can utilize different metrics to adapts its selection strategy.



Now that we are briefly familiar with each components, I will dive deeper in each components by describing their subprocesses and their interactions with each other.

### Experiment


#### Experiment Setup

#### Test Strategy

#### Data Strategy

### Journal

#### Selection Strategy

#### Submission


### Researcher

#### Hacking Strategy

#### Decision Strategy
