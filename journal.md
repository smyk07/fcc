# Journal

This is a learning project for me, for learning IR building and Optimization Passes.

Hoping to keep daily Dev log like reports in this file. Days in decending order.

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
