---
parent: API
---

HackingStrategy Class Reference
===============================

HackingStrategy

Abstract class for Hacking Strategies.

`#include <HackingStrategies.h>`

-   virtual void **perform** (Experiment \*experiment, DecisionStrategy
    \*decisionStrategy)=0

<!-- -->

-   static HackingStrategy \* buildHackingMethod (json &config)

Detailed Description
--------------------

Abstract class for Hacking Strategies.

Each *Hacking Strategy* should provide a `perform()` method. The
`perform()` method will take over a **pointer** copy of an Experiment
and apply the implemented hacking on it. Changes will be applied to the
copy and will be return to the Researcher.

> **Note**
>
> Researcher decides if it\'s going to pass a copy or a reference to an
> existing Experiment. This is important since it allows the researcher
> to either stack different hacking over each others or pass a new copy
> --- usually the original experiment --- to the experiment.

Member Function Documentation
-----------------------------

### buildHackingMethod()

buildHackingMethod

HackingStrategy

HackingStrategy

buildHackingMethod

`HackingStrategy * HackingStrategy::buildHackingMethod (json & config)[static]`

A Factory method for building hacking strategies

**See also:.**

README.md

**Parameters.**

  ------------- --------------------------------------------------------
  config        A JSON object defining a hacking strategy, and its
                parameters

  ------------- --------------------------------------------------------

**Returns:.**

Pointer to a HackingStrategy

The documentation for this class was generated from the following files:

include/HackingStrategies.h

src/HackingStrategies.cpp
