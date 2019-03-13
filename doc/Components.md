<div style="text-align: right"><i>Design</i></div><br>

In the previous section, while reviewing the underling process of conducting a scientific research, I listed some of the main components and entities involving in different stages of a research, e.g., *Experiment Setup, Experiment, Researcher, Submission*, and *Journal*. In the abstraction, each component  can be seen as a loosely independent entity while the whole system, i.e., *scientific research*, can be described by a set of behaviors, tasks and interactions.

One of our main design goals with SAM was to achieve a level of flexibility where we could modify different aspects of the research process relatively easily. In order to achieve this,  we have tried to decouple the system to smaller but conceptually meaningful scientific routines and objects. Figure 1. shows all the components of SAM and their dependencies alongside their interactions with each other. 

This section will clarify the design principles behind each component, how they work and interact with each other in order to simulate the entire process.

> Figure 1.

## SAM’s Main Components

SAM consists of 3 main components, *Experiment, Researcher* and *Journal*. Each component tries to summarize one of the mentioned subprocesses or entities of a scientific research as described in the [Introduction](Introduction.md). The list below briefly introduces each component and its task and role.

- The [**Experiment**](#experiment) consists of several subroutines and objects dealing with different aspect of a research, e.g., setup, data, test. 
	- [*ExperimentSetup*](#experiment-setup) holds the necessary information and parameters of how the experiment should be defined. Parameters of the ExperimentSetup cannot be modified during the simulation and only can be set at the start of the experiment. In fact, it resembles the *pre-registration*.
	- [*Data Strategy*](#data-strategy) is a routine used to generate the necessary data based on the parameters specified in *ExperimentSetup*.
	- [*Test Strategy*](#test-strategy) is a statistical method of choice in the Experiment Setup for testing the result of the experiment.
- The **[Researcher](#researcher)** object imitates the behaviors of a researcher including his possible questionable research practices. The researcher will define the *ExperimentSetup*, generate/collect the data, run the statistical test, decides whether to preform any QRPs, prepare the *Submission* record, and finally submit it to his *Journal* of choice.
    - [*Decision Strategy*](#decision-strategy) is the underling logic that the researcher uses for selecting an outcome variable for submission.
    - [*Hacking Strategies*](#hacking-strategy) is a list of questionable research practices in researcher's hand in case he decides to hack his way through finding significant results. 
- The **[Journal](#journal)** is a container of *Submission*s, i.e., published studies. The Journal keeps track of its publications and can utilize different metrics to adapts its selection strategy.
    - [*Selection Strategy*](#selection-strategy) is an internal algorithm/criteria of a journal to decided whether a submission is going to be accepted or not. 

After this brief introduction to each component, I will dive deeper in each of them to describe their subprocesses, properties and relations.

SAM utilizes several object-oriented principles and design patterns to achieve the level of flexibility that is offering. Since all comments of SAM are technically  C++ `class`, from now on, I’ll refer to them as objects, e.g., Experiment object. Moreover, I’ll use monospace font to refer to them as this helps distinguishing them from the rest of the story. 

### Experiment

As mentioned, `Experiment` object acts as an umbrella object for everything related to the an actual experiment. This includes meta data, raw data, method/model for generating the data, e.g., linear model, and method of testing the hypothesis. The `Researcher` object has complete control to every aspects of an `Experiment` with one exception, it can only read but not modify the `ExperimentSetup` object. This is an important factor when we are going to implement the concept of pre-registration.

While the full list of `Experiment` parameters’ and method can be found here (link to the Doxygen page), a short list provided below summarize some of the important variables and methods.



#### Experiment Setup

SAM treats the `ExperimentSetup` object as readonly object after the initialization phase. During the initialization phase, SAM initialize and randomize the `ExperimentSetup` based on the listed parameters in the configuration file (link to the section). After the initialization phase, `ExperimentSetup` will persist intact in the code and will be used as a reference point in different stages. For instance, if the `Journal` object is designed such that it requires a research to be pre-registrated, `Journal` can access the `ExperimentSetup` and make its decision accordingly. 

Some of the parameters of ExperimentSetup are:

#### Data Strategy

`DataStrategy`

#### Test Strategy

`TestStrategy` is an object that provides the routine for testing the hypothesis. TestStrategy has access to the entire Experiment and can modify some of its variables, e.g., `pvalue, statistics`, etc. 

Currently t-test is the only TestStrategy provided by SAM but we plan to add more methods to the pool. T-test needs to know the sides of the test, whether variances are equal and the value of $\alpha$. These parameters can be set using the `—test-strategy` section of configuration file. (Link to the section)



### Journal

#### Selection Strategy

#### Submission


### Researcher

#### Hacking Strategy

#### Decision Strategy
