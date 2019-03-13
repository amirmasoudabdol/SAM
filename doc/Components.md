## SAM’s Design

Computer simulations are one of the effective tools that helps us mimic complicated, expensive and time consuming systems *in silico*, in order to move toward better understanding of interactions and hopefully formulate new theories that can be tested in real life.

Any computer program attempting to simulate this complicated process, ...



## SAM’s Main Components

SAM consists of 3 main parts, *Experiment, Researcher* and *Journal*. 

- The *[Experiment](#experiment)* contains all the necessary data and parameters for defining and modeling a specific experiment. *ExperimentSetup* is an object inside the Experiment acting as a container for all the parameters. Parameters in the ExperimentSetup cannot be changed and only be set at the start of the experiment. In another word, they act as they are pre-registered experiment. 
- The *Researcher* imitates the behaviors of a researcher and includes his QRP’s preferences. A researcher will collect the data for an Experiment based on the parameters specified in ExperimentSetup. It’ll then run the test, decides which outcome it’s like to submit to the Journal. A researcher action can be altered by his choice of p-hacking method. 
- The *Journal* defines the behavior of the journal or a research track when it comes to reviewing or accepting a publication. 


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
