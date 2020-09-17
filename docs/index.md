---
title: Home
layout: default
---

SAM
===

SAM is an extensible *p*-hacking simulator. It provides different
interfaces and APIs for designing, and experimenting with different
statistical models under the influence of various *p*-hacking strategies
as well as other types of *questionable research practices*, QRPs. For
instance, one could design a two-by-two factorial experiment and study
the effect of optional stopping on achieving significant results. SAM
offers number of conventional and recognized *p*-hacking methods out of
the box; in addition, a user is able to implement new methods according
to his/her need, e.g., a specific type group pooling.

In this vignette, I'll describe underlying design principles and list of
the main features of SAM alongside some examples. In the
`introduction`{.interpreted-text role="doc"} section, I'll describe type
of problems that SAM is trying to solve and how are we going to approach
it. After setting the ground rules, we can dive into different
components of SAM and the process of designing your own simulation.

::: {.toctree maxdepth="2"}
introduction design flow configuration-file data-strategies
test-strategies effect-strategies hacking-strategies decision-strategies
selection-strategies
:::
