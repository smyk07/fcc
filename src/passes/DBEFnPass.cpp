/*
 * DBE: Dead Block Elimination
 *
 * Eliminates unreachable blocks
 */

#include "passes/DBEFnPass.hpp"
#include "IR.hpp"

#include <unordered_set>
#include <vector>

namespace fcc {

bool DBEFnPass::run(Function &fn) {
  std::vector<BasicBlock *> worklist;
  std::unordered_set<BasicBlock *> live;

  worklist.push_back(fn.blcks[0].get());
  live.insert(fn.blcks[0].get());

  while (!worklist.empty()) {
    auto bb = worklist.back();
    worklist.pop_back();

    for (auto sbb : bb->successors()) {
      if (!live.contains(sbb)) {
        worklist.push_back(sbb);
        live.insert(sbb);
      }
    }
  }

  auto old_size = fn.blcks.size();
  bool changed = false;

  std::erase_if(fn.blcks, [&live](auto &bb) {
    auto bb_ptr = bb.get();

    if (!live.contains(bb_ptr)) {
      auto sbbs = bb_ptr->successors();

      for (auto sbb : sbbs) {
        std::erase_if(sbb->preds,
                      [&bb_ptr](auto pbb) { return pbb == bb_ptr; });

        for (auto &si : sbb->instrs) {
          if (si->op != OpCode::Phi)
            continue;

          auto &incoming = std::get<PhiData>(si->payload).incoming;

          std::erase_if(incoming,
                        [&bb_ptr](auto pair) { return pair.first == bb_ptr; });
        }
      }

      return true;
    }

    return false;
  });

  changed |= fn.blcks.size() != old_size;

  return changed;
}

} // namespace fcc
