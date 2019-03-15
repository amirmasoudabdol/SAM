---
date: Fri Mar 15 2019
section: 3
title: Journal
---

NAME
====

Journal

SYNOPSIS
========

\

Public Member Functions
-----------------------

**Journal** (json &config)\

**Journal** (double max\_pubs, double pub\_bias, double alpha)\

void **setSelectionStrategy** (**SelectionStrategy** \*s)\

bool **review** (**Submission** &s)\

void **accept** (**Submission** s)\

void **reject** (**Submission** &s)\

void **clear** ()\

bool **isStillAccepting** ()\

void **saveSubmissions** (int simid, std::ofstream &writer)\

Public Attributes
-----------------

std::vector\< **Submission** \> **submissionList**\

**SelectionStrategy** \* **selectionStrategy**\

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
