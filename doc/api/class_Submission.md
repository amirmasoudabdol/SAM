---
parent: API
---

Submission Class Reference
==========================

Submission

-   **Submission** (Experiment &e, const int &index)

-   const bool isSig () const

<!-- -->

-   int simid = 0

    Simulation ID.

-   int pubid = 0

    Publication ID.

-   int **inx**

-   int nobs

    Number of observation in submitted group.

-   double yi

    Effect size of the submitted group.

-   double sei

    Standard error of the submitted group.

-   double statistic

    Corresponding statistics of the submitted group.

-   double pvalue

    *P*-value of the submitted group

-   bool sig = false

    Indicates if the submission is significant or not.

-   short side = 1

    The side of the observed effect.

-   bool **isHacked** = false

-   int **tnobs**

-   double tyi

    True mean/effect of the selected submission record.

-   double **tvi**

-   double **tcov**

-   double **pubbias**

<!-- -->

-   std::ostream & **operator\<\<** (std::ostream &os, const Submission
    &s)

Member Function Documentation
-----------------------------

### isSig()

isSig

Submission

Submission

isSig

`const bool Submission::isSig ( ) const[inline]`

**Returns:.**

`true` if the Submission is significant, `false` otherwise

The documentation for this class was generated from the following file:

include/SubmissionRecord.h
