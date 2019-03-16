---
parent: API
---

Journal Class Reference
=======================

Journal

-   **Journal** (json &config)

-   **Journal** (double max\_pubs, double pub\_bias, double alpha)

-   void **setSelectionStrategy** (SelectionStrategy \*s)

-   bool **review** (Submission &s)

-   void **accept** (Submission s)

-   void **reject** (Submission &s)

-   void **clear** ()

-   bool **isStillAccepting** ()

-   void **saveSubmissions** (int simid, std::ofstream &writer)

<!-- -->

-   std::vector\< Submission \> **submissionList**

-   SelectionStrategy \* **selectionStrategy**

The documentation for this class was generated from the following files:

include/Journal.h

src/Journal.cpp
