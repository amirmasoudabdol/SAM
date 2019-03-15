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

int **pubid** = 0\

int **inx**\

int **nobs**\

double **yi**\

double **sei**\

double **statistic**\

double **pvalue**\

bool **sig** = false\

short **side** = 1\

bool **isHacked** = false\

int **tnobs**\

double **tyi**\

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
