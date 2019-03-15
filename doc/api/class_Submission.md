---
date: Fri Mar 15 2019
parent: API
section: 3
title: Submission
---

NAME
====

Submission

SYNOPSIS
========

\

Public Member Functions
-----------------------

**Submission** (**Experiment** &e, const int &index)\

const bool **isSig** () const\

Public Attributes
-----------------

int **simid** = 0\
Simulation ID.

int **pubid** = 0\
Publication ID.

int **inx**\

int **nobs**\
Number of observation in submitted group.

double **yi**\
Effect size of the submitted group.

double **sei**\
Standard error of the submitted group.

double **statistic**\
Corresponding statistics of the submitted group.

double **pvalue**\
*P*-value of the submitted group

bool **sig** = false\
Indicates if the submission is significant or not.

short **side** = 1\
The side of the observed effect.

bool **isHacked** = false\

int **tnobs**\

double **tyi**\
True mean/effect of the selected submission record.

double **tvi**\

double **tcov**\

double **pubbias**\

Friends
-------

std::ostream & **operator\<\<** (std::ostream &os, const **Submission**
&s)\

Member Function Documentation
=============================

const bool Submission::isSig () const` [inline]`
------------------------------------------------

**Returns:**

> `true` if the **Submission** is significant, `false` otherwise

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
