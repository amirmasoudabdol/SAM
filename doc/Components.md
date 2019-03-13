## SAM’s Design

In the previous section, I have tried to extract main components and entities involving in different stages of the research, e.g., *Experiment Setup, Experiment, Researcher, Submission*, and *Journal*. In the abstraction, each component  can be seen as a loosely independent entity while the whole system, i.e., *scientific research*, can be described by set of behaviors, tasks and interactions. 

During the design process of SAM, we have tried to decompose the entire process into as small as possible programmable units while making sure that their roles and interactions is capable of mimicking the entire process as a whole. Figure 1 shows each component of SAM, and their interactions.

> Figure 1.

Our goal was ...

The rest of this section will dive deeper into the design principle and sub-entities of each components.

### SAM’s Main Components

SAM consists of 3 main components, *Experiment, Researcher* and *Journal*. Each component tries to summarize one of the mentioned subprocesses or entities of a scientific research as described in the [Introduction](Introduction.md). The list below briefly introduces each component and its task and role.

- The [*Experiment*](#experiment) contains all the necessary data and parameters for defining and modeling a specific experiment. *ExperimentSetup* is an object inside the Experiment acting as a container for all the parameters. Parameters in the ExperimentSetup cannot be changed and only be set at the start of the experiment. In another word, they act as they are pre-registered experiment. 
- The *Researcher* imitates the behaviors of a researcher and includes his QRP’s preferences. A researcher will collect the data for an Experiment based on the parameters specified in ExperimentSetup. It’ll then run the test, decides which outcome it’s like to submit to the Journal. A researcher action can be altered by his choice of p-hacking method. 
- The *Journal* defines the behavior of the journal or a research track when it comes to reviewing or accepting a publication. 

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
