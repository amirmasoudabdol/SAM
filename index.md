---
layout: default
title: Home
nav_order: 1
---

SAM is an extensible p-hacking simulator. It provides different interfaces and APIs for designing — and experimenting with — different statistical experiments under the influence of various *p*-hacking methods or strategies, a.k.a *questionable research practices*. For example, one could design a two-by-two factorial experiment and study the effect of optional stopping on achieving significant results.

While there are a few conventional and well-known p-hacking methods are provided out of the box, the flexibility of SAM allows the user to define any specific methods, e.g., modifying measurements based on an specific metric, and apply it on the currently existing experiment.

In this vignette, I’ll describe the underlying design principles of SAM and some of its capabilities alongside some examples. In the Introduction section, I’ll explain what’s the problem that SAM is trying to solve and how are we going to approach it. After setting the ground roles, we can dive into different components of SAM and process of designing your own simulation. 