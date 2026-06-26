# Journal

This is a learning project for me, for learning IR building and Optimization Passes.

Hoping to keep daily Dev log like reports in this file. Days in decending order.

## 26/06/2026

- made a coding style change, using anonymous namespace instead of static, I carried over my habits from C.

## 25/06/2026

- Implemented Constant folding today, it was fun.
- man i keep updating my test script like every day, but its okay

## 24/06/2026

- Lowered for loop today, and unexpectedly got it at the first try lesgooooooo.
- although i do need to handle some edge cases, such as empty CompountStmt, and an BinaryOperator = instead of a VarDecl in loop init.

## 23/06/2026

- Lowered break and continue today. This also exposed a condition in tryRemoveTrivialPhi where a dangling pointer would be left in the defs map which caused use after frees later.

## 22/06/2026

- Lowered while loop today. Found another bug where if successors is called on a block during lowering (through predecessors) before it is fully populated, the function could hit a non-terminating instruction and crash. Better to cache predecessors while sealing.

> for loop tmrw?

## 21/06/2026

- Lowered conditional oparators, also found out clang gives you these ParenExpr wrappers for (...) like expressions. lowered that too.
- lowered ifstmt and found out how buggy my code was :sob:
- also better printing of function params
- added basic pass infra, but i ended up overcomplicating it, unordered_map for everything is bad so i took help of chatgpt to help me design a fancy constexpr registry struct for storing passes
- implemented DDE lesgo
- also updated the test runner to run differently for pass\_\* tests

> tomorrow will hopefully lower while loop

## 20/06/2026

- Added basic testing infra
- Added vardecl lowering
- also added assignment lowering
- also added a lot of helpers since i finally read the Braun et al. paper, im sure it will have some bugs tho

> tomorrow i will hopefully lower comparison ops and if-elseif-else stmts
> and addPhiOperands and tryRemoveTrivialPhi

## 19/06/2026

- Improved the way functions are printed by IRPrinter
- Fixed the way types were handled while lowering, the types are now cached. Still immature but its good enough for this stage.
- lowered binary operators

## 18/06/2026

- Created minimal IR definitions, will need more additions to it later but lets start with lowering a subset of c99 right now.
- Created IRPrinter

## 17/06/2026

- Made a clang frontend wrapper
