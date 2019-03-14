<div style="text-align: right"><i>Flow</i></div><br>

SAM simulate the scientific process through several subprocesses as depicted in the following flowchart. Each rounded rectangle describes a process, often perform by one of the components or through collaboration between different components. This section describes each subprocess in more details. 

<center>
![](/figures/main-routine.svg)
</center>

## Initialization

During the initialization phase, SAMs loads a given configuration file and initializes different compartments based on the given parameters. The specification of the configuration file can be found [here](ConfigurationFileSpecifications.md). 

After this stage, every component is defined based on user’s preference and SAM can initiate the scientific research process. 

<center>
![](/figures/initialization.svg)
</center>

## Prepare Research

Research preparation is the first in the simulation. This step resembles the process of defining a study and finally collecting the data for the study. As we discussed in the [Design](#Components.md) section, **`ExperimentSetup`** is an readonly object except the case that a user prefers to randomize one of its parameters in each different runs of SAM, for instance, randomizing the number of observations in order to study its effect on a particular metric. This can be done by setting `nobs` parameters to “random”. In this case, SAM first randomizes the `ExperimentSetup` before making it a readonly object. 

The next step is allocating memory for the data, and analysis through the simulation. And finally, Experiment uses its already initialized **`DataStrategy`** to draw a sample from the given population or distribution. This will populate the `measurements` variable and SAM is ready to move to the next process. 

<center>
![](/figures/prepare-research.svg)
</center>

## Perform Research

Performing the research can be closely tied to the data processing process. At this stage, `Researcher` uses the **`TestStrategy`** of the `Experiment` to run the test and populated parameters like `statistic, pvalue, sig, side`, etc.  

The next step is to check whether the Researcher is satisfied with test results or not. This is being done by passing the **`Experiment`** to the `**DecisionStrategy**. Researcher` relies on DecisionStrategy’s verdict to decide if it’s going to proceed with the Submission or should it apply one or more hacking methods on the experiment before submitting the experiment for the review. As mentioned, Researcher can have access to a list of hacking methods. For each hacking method, ***h***, in its arsenal, a copy of Experiment is passed to the hacking method. Based on the hacking method, researcher could ask `DecisionStrategy` whether it should continue or stop. For instance, in the case of optional stopping, Researcher can decide after each addition whether its satisfied or not, *intermediate verdict.*

<center>
![](/figures/perform-research.svg)
</center>

## Publish Research

Process of publishing the research as it sounds resembles the preparation of final Submission and sending it to the Journal for review. 

After applying the last hack, `Researcher` will proceed with preparation of its **final** Submission, by asking `DecisionStrategy` for its *final verdict*. This is were `Researcher` gets the opportunity to look back at different versions of `Experiment` or `Submission` and pick the most satisfying record.  

At this stage, despite finding any significant results or not, researcher **must** prepare a Submission record and submit it to the `Journal` for review. The submitted Submission will be judged by `Journal`’s `SelectionStrategy` and either gets accepted or rejected. If accepted, `Journal` will add the submission to its publicationsList for further analysis or possible adjustment of its `SelectionStrategy`. 

<center>
![](/figures/publish-research.svg)
</center>
