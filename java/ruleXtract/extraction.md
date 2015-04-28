Rule Extraction Logic
=============================

In this document we describe the heuristics used in the rule extraction logic. In particular we focus on how the rule extractor differs from Koehn and Chiang

### Input:

+   Source Sentence
+   Target Sentence
+   Word alignments
+   The *MaxSourcePhrase* parameter (Integer)
+   The *MaxSourceElements* parameter (Integer)
+   The *MaxTerminalLength* param (Integer)
+   The *RemoveMonotonicRepeats* param (Boolean)

### Output: 
+   A sequence of hiero rules. It may include duplicates

## Phrase Extraction:

A phrase pair is a subsequence of the source and a subsequence of the target that obey the constraints described by Koehn. The phrase pair is defined with respect to spans of the phrases, not their content. For example in the following fully aligned parallel sentence:

source:
3_4_5_3_4_5
target:
7_8_9_7_8_9

The phrase 3_4_5-7_8_9 will be included twice because one phrase pair spans the first three words, and the second phrase pair spans the second three words. Let us define the following sets of phrase pairs

+ **ALL** The set of all phrase pairs that can be extracted from a sentence. Because no length constraints are used this set includes the parallel sentence itself
+ **PARENTS** The **ALL** set filtered such that the boundary words of the target phrase must be aligned, the last word of the source phrase is aligned, and the first word of the source phrase must be aligned if it is the start of the sentence
+ **CHILDREN** The **ALL** set filtered such that all source words must be aligned, and the length of the source phrase must be less or equal to *MaxTerminalLength*. The target is allowed to have unaligned words
+ **PHRASE** The **ALL** set filtered such that the length of the source phrase must be less or equal to *MaxSourcePhrase*

## Hiero Rule Extraction

The **PARENTS** set of phrase pairs is used to form the hiero rules. Non-terminals are identified by finding constituent phrase pairs that are contained in the **CHILDREN** set.

Let us define a function called extractNT that given a set of phrase pairs or rules returns a set of rules. Each rule in this set is a transformed input phrase pair or rule where one, and only one, constituent phrase pair has been replaced in the parent with a non-terminal. 

The extractNT function is invoked twice. In the first invokation the resulting list is a set of rules with one non-terminal, which we call **ONE_NT**. Any rule where the source or target string is completely spanned by a non-terminal is filtered from the set. Examples of these filtered rules include

+   3_X-X
+   X-X_9
+   X-X 

The **ONE_NT** list is used as the input into the second invokation of the extractNT function. The result is a set of rules with two non-terminals, which we call **TWO_NT**. If *RemoveMonotonicRepeats* is true then monotonic repeats are filtered from the rules. A monotonic repeat is a rule where the X1 and X2 symbols are grouped consecutively in the source or target phrase. The following are examples of monotonic repeats:

+   3_X1_X2-X1_8_X2
+   X1_4_X2_3-7_X1_X2
+   X1_X2_2-9_X1_X2

The **TWO_NT** list is additionally filtered.

### Non-terminal filtering

Hiero rules are subject to additional filtering:

   + The length of the source string of the rule, including non-terminals, must be less or equal to *MaxSourceElements*
   + The number of terminals in the source string of the rule must be less or equal to *MaxTerminalLength*

The **TWO_NT** list is automatically filtered. However, the  **ONE_NT** cannot be filtered because it is used as input to extractNT. We therefore define a second set **ONE_NT_FILTERED** which is **ONE_NT** after this filtering has been applied.

## Duplicate Rules

The function extractNT generates duplicate rules. Two rules are duplicates if the source and target strings are identical, even if they cover different spans. Many duplicates are generated if the non-terminal occurs at the start or end of rule. For example consider the following **PARENT** and **CHILDREN** sets.

**PARENT** = { 3_4_5-7_8_9, 1_2-7_8 }

**CHILDREN** = { 3_4-7_8, 4-8 }

We can extract the rule 3_X-7_X twice using 3_4_5-7_8_9 with the constituent 4_5-8_9, and using 3_4-7_8 with the constituent 4-8. The duplicate logic is as follows

   + Duplicates phrase pairs that span different parts of the sentence are preserved
   + Duplicate one non-terminal rules that result from duplicate parent phrase pairs are preserved
   + Duplicate one non-terminal rules due to non-terminals that occur at the start or end of the source string are filtered
   + Duplicate two non-terminal rules where the non-terminals are at both the start and end of the source string are filtered, e.g. duplicates of X1_3_X2-X2_8_X3 would be filtered
   + Duplicate two non-terminal rules due to non-terminals that occur either at the start or end of the source string, but not both, are preserved. E.g duplicates of X1_4_X2_3-X1_7_X2_8 would be preserved.

## Final Output

The final output of the rule extractor is the union of **PHRASE**, **ONE_NT_FILTERED**, and **TWO_NT**


