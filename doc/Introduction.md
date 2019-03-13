<div style="text-align: right"><i>Introduction</i></div>
<br>

Before I start discussing SAM’s implementation and design, let’s review the underlying process of defining, conducting and reporting a common scientific research. In the next section, I will try to explain an — embarrassingly — simplified process of producing a scientific result. This will allow me to expose typical components and entities that are involved in the each stage, and later on explain how each subprocess translates into different components of SAM. At the end, I will briefly touch up on the question of why we decided to develop SAM the way we did.

## Scientific Research Process

The *process* of producing a scientific research/study is often a cumbersome and complicated process. Despite all different ways of doing research, a typical scientific study starts by an formulating a hypothesis as the [**Researcher**](Components.md#researcher) jots down his ideas on how a process explain a certain phenomena or how a complicated system works. In order to test his hypothesis, he/she *setup an experiment* by while he/she describes the underlying parameters, compromises or criteria of the study.

After the [**Experiment Setup**](Components.md#experiment-setup) is finalized, the [**Experiment**](Components.md#experiment) is being conducted in which the researcher collects a set of datasets summarizing measurements, censors data, images, etc. required to test his/her hypothesis. The next step is a mix of processing the data, analyzing the data and coming up with a conclusion regarding the initial hypothesis regardless of agreeing or disagreeing with the initial hypothesis, *pre-registered hypothesis*.

If the result is considered satisfactory and informative, despite the correctness of the initial hypothesis, the researcher will select a [**Journal**](Components.md#journal) and submit his research in the form of a [**Publication**](Components.md#submission) to be reviewed by journal’s criteria. Finally, the Journal will decide if the submitted result worth publishing or not. 

### Meta-analysis

As it’s been shown and discussed [cite, cite], this process is often long and prune to many errors in every different stage. It’s natural complexity as well as large numbers of degrees of freedom in the hand of Researchers and Journals will affect the quality of research and publications. Researchers —intentionally or unintentionally — may make mistakes and Journals may select particular set of publications based on wrong or outdated criteria and as a result resources will be wasted and our selected publications might be biased one way or another. 

Studying this complicated interaction between a *Research*, his/her *Researcher*, the *Journal*, i.e. publishing medium, and finally the outcome, *Knowledge*, is proven to be a challenging task [cite]. In recent years, the field of meta-analysis has significantly grown in popularity in order to try to evaluate and aggregate our collective knowledge of certain hypotheses. As a result, we discovered the effect of researcher’s degrees of freedoms on published results [cite] and journal’s bias toward “positive and novel” results on our knowledge [cite]. It’s been shown that process of adding new data points to an experiment — after the initial data collection — is more wide-spread and has more severe effects on researchers ability to achieve significant results, and therefore makes his/her research more appealing to biased journals. Therefore, the aggregated outcome, *published results*, of a specific theory will be skewed toward incorrect conclusions. 

Unfortunately, studying and understanding effects of some of the other questionable research practices, QRPs, are not often as straightforward or apparent as effects of *optional stopping*. The list of degrees of freedoms is long [cite], and researchers are not fully aware of their consequences on the final outcome of their research. At the same time, the scientific community is also not fully understand (or agree on) the propagative impacts these practices on our collective knowledge, *true effect size*.


Next: [SAM's Design and Components](Components.md)