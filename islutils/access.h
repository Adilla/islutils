#ifndef ACCESS_H
#define ACCESS_H

#include <isl/cpp.h>

#include <algorithm>
#include <functional>
#include <vector>

#include "islutils/die.h"
#include "islutils/operators.h"

namespace matchers {

// Candidates are parameterized by the type of Payload they carry.  Only one
// type of payload is allowed within a PlaceholderSet.
// Instances of payload must be comparable to each other.
template <typename Payload> class DimCandidate {
public:
  DimCandidate(const Payload &payload, isl::space candidateMapSpace)
      : payload_(payload), candidateMapSpace_(candidateMapSpace) {}

  bool isEqualModuloMap(const DimCandidate &other) const {
    return payload_ == other.payload_;
  }

  Payload payload_;

  // The space of the candidate map.  It is sufficient to uniquely identify the
  // map in the union_map the matcher operates on.
  isl::space candidateMapSpace_;
};

template <typename CandidatePayload, typename PatternPayload>
class UnpositionedPlaceholder {
public:
  explicit UnpositionedPlaceholder(PatternPayload pattern)
      : pattern_(pattern), candidates_({}), id_(nextId_++) {}
  UnpositionedPlaceholder(const UnpositionedPlaceholder &) = default;

  PatternPayload pattern_;
  std::vector<DimCandidate<CandidatePayload>> candidates_;

  const size_t id_;

private:
  static thread_local size_t nextId_;
};

template <typename CandidatePayload, typename PatternPayload>
thread_local size_t
    UnpositionedPlaceholder<CandidatePayload, PatternPayload>::nextId_ = 0;

template <typename CandidatePayload, typename PatternPayload>
class Placeholder
    : public UnpositionedPlaceholder<CandidatePayload, PatternPayload> {
public:
  explicit Placeholder(PatternPayload pattern, int pos)
      : UnpositionedPlaceholder<CandidatePayload, PatternPayload>(pattern),
        outDimPos_(pos) {}
  explicit Placeholder(
      const UnpositionedPlaceholder<CandidatePayload, PatternPayload> &other,
      int pos)
      : UnpositionedPlaceholder<CandidatePayload, PatternPayload>(other),
        outDimPos_(pos) {}

  int outDimPos_;
};

template <typename CandidatePayload, typename PatternPayload>
inline Placeholder<CandidatePayload, PatternPayload>
dim(int pos, UnpositionedPlaceholder<CandidatePayload, PatternPayload> ph) {
  return Placeholder<CandidatePayload, PatternPayload>(ph, pos);
}

template <typename CandidatePayload, typename PatternPayload>
using PlaceholderList =
    std::vector<Placeholder<CandidatePayload, PatternPayload>>;

template <typename Arg, typename Arg0, typename... Args>
struct all_are
    : public std::integral_constant<bool, std::is_same<Arg, Arg0>::value &&
                                              all_are<Arg, Args...>::value> {};

template <typename Arg, typename Arg0>
struct all_are<Arg, Arg0>
    : public std::integral_constant<bool, std::is_same<Arg, Arg0>::value> {};

// Placeholder<CandidatePayload, PatternPayload> is used twice in all_are to
// properly handle the case of sizeof...(Args) == 0 (all_are is not defined for
// 1 argument).
template <typename CandidatePayload, typename PatternPayload, typename... Args>
typename std::enable_if<
    all_are<Placeholder<CandidatePayload, PatternPayload>,
            Placeholder<CandidatePayload, PatternPayload>, Args...>::value,
    PlaceholderList<CandidatePayload, PatternPayload>>::type
access(Placeholder<CandidatePayload, PatternPayload> arg, Args... args) {
  return {arg, args...};
}

template <typename CandidatePayload, typename PatternPayload, typename... Args>
typename std::enable_if<
    all_are<UnpositionedPlaceholder<CandidatePayload, PatternPayload>,
            UnpositionedPlaceholder<CandidatePayload, PatternPayload>,
            Args...>::value,
    PlaceholderList<CandidatePayload, PatternPayload>>::type
access(UnpositionedPlaceholder<CandidatePayload, PatternPayload> arg,
       Args... args) {
  PlaceholderList<CandidatePayload, PatternPayload> result;
  int pos = 0;
  for (const auto &a : {arg, args...}) {
    result.emplace_back(a, pos++);
  }
  return result;
}

template <typename CandidatePayload, typename PatternPayload>
class PlaceholderSet;

template <typename CandidatePayload, typename PatternPayload, typename... Args>
PlaceholderSet<CandidatePayload, PatternPayload> allOf(Args... args);

template <typename CandidatePayload, typename PatternPayload>
class PlaceholderSet {
  // TODO: Check if we can friend a partial specialization
  template <typename CPayload, typename PPayload, typename... Args>
  friend PlaceholderSet allOf(Args... args);

public:
  std::vector<Placeholder<CandidatePayload, PatternPayload>> placeholders_;

  // Each inner vector has a set of indices of placeholders that should appear
  // together in a relation.  Different groups must correspond to different
  // relations.  We store indices separately because a placeholder may appear
  // in multiple relations, actual objects are stored in placeholders_.  We
  // don't store references because of how the match is currently structured: a
  // vector of candidates, each of which is itself a vector with the same index
  // as the position of the placeholder in placeholders_.  This may change in
  // the future for a more C++-idiomatic API.
  std::vector<std::vector<size_t>> placeholderGroups_;

  // Placeholder fold is an identifier of a set of placeholders that must get
  // assigned the same candidate value modulo the matched map.  The idea is to
  // reuse, at the API level, placeholders in multiple places to indicate
  // equality of the matched access patterns.
  // This vector is co-indexed with placeholders_.  By default, each
  // placeholder gets assigned its index in the placeholders_ list, that is
  // placeholderFolds_[i] == i. Placeholders that belong to the same group have
  // the same fold index, by convention we assume it is the index in
  // placeholders_ of the first placeholder in the fold.
  // One placeholder cannot belong to multiple folds.
  std::vector<size_t> placeholderFolds_;
};

/// Build an object used to match all of the access patterns provided as
/// arguments. Individual patterns can be constructed by calling "access(...)".
template <typename CandidatePayload, typename PatternPayload, typename... Args>
PlaceholderSet<CandidatePayload, PatternPayload>
allOf(PlaceholderList<CandidatePayload, PatternPayload> arg, Args... args) {
  static_assert(all_are<PlaceholderList<CandidatePayload, PatternPayload>,
                        PlaceholderList<CandidatePayload, PatternPayload>,
                        Args...>::value,
                "can only make PlaceholderSet from PlaceholderLists "
                "with the same payload types");

  std::vector<PlaceholderList<CandidatePayload, PatternPayload>>
      placeholderLists = {arg, args...};
  std::vector<std::pair<size_t, size_t>> knownIds;
  PlaceholderSet<CandidatePayload, PatternPayload> ps;
  for (const auto &pl : placeholderLists) {
    if (pl.empty()) {
      continue;
    }

    size_t index = ps.placeholders_.size();
    ps.placeholderGroups_.emplace_back();
    for (const auto &p : pl) {
      ps.placeholders_.push_back(p);
      ps.placeholderGroups_.back().push_back(index);
      auto namePos = std::find_if(knownIds.begin(), knownIds.end(),
                                  [p](const std::pair<size_t, size_t> &pair) {
                                    return pair.first == p.id_;
                                  });
      if (namePos == knownIds.end()) {
        knownIds.emplace_back(p.id_, index);
        ps.placeholderFolds_.emplace_back(index);
      } else {
        ps.placeholderFolds_.emplace_back(namePos->second);
      }
      ++index;
    }
  }

  return ps;
}

template <typename CandidatePayload, typename PatternPayload> class Match {
public:
  Match(const PlaceholderSet<CandidatePayload, PatternPayload> &ps,
        const std::vector<DimCandidate<CandidatePayload>> &combination);

  DimCandidate<CandidatePayload> operator[](
      const UnpositionedPlaceholder<CandidatePayload, PatternPayload> &pl)
      const {
    auto result = std::find_if(
        placeholderValues_.begin(), placeholderValues_.end(),
        [pl](const std::pair<size_t, DimCandidate<CandidatePayload>> &kvp) {
          return kvp.first == pl.id_;
        });
    if (result == placeholderValues_.end()) {
      ISLUTILS_DIE("no match for the placeholder although matches found");
    }
    return result->second;
  }

private:
  std::vector<std::pair<size_t, DimCandidate<CandidatePayload>>>
      placeholderValues_;
};

template <typename CandidatePayload, typename PatternPayload>
using Matches = std::vector<Match<CandidatePayload, PatternPayload>>;

template <typename CandidatePayload, typename PatternPayload>
Matches<CandidatePayload, PatternPayload>
match(isl::union_map access,
      PlaceholderSet<CandidatePayload, PatternPayload> ps);

template <typename CandidatePayload, typename PatternPayload>
struct Replacement {
  Replacement(PlaceholderList<CandidatePayload, PatternPayload> &&pattern_,
              PlaceholderList<CandidatePayload, PatternPayload> &&replacement_)
      : pattern(pattern_), replacement(replacement_) {}

  PlaceholderList<CandidatePayload, PatternPayload> pattern;
  PlaceholderList<CandidatePayload, PatternPayload> replacement;
};

template <typename CandidatePayload, typename PatternPayload>
Replacement<CandidatePayload, PatternPayload>
replace(PlaceholderList<CandidatePayload, PatternPayload> &&pattern,
        PlaceholderList<CandidatePayload, PatternPayload> &&replacement) {
  return {std::move(pattern), std::move(replacement)};
}

// Calls like this do not fully make sense: different replacements for
// essentially the same pattern.
// makePSR(replace(access(_1, _2), access(_2, _1)),
//         replace(access(_3, _4), access(_3, _4)))
// They could become useful if access is further constrained to specific arrays
// or statements/schedule points.
//
// Calls like this contain redundant information and should be disallowed.
// makePSR(replace(access(_1, _2), access(_2, _1)),
//         replace(access(_1, _2), access(_2, _1)))
//
// Generally, we should disallow such transformations that affect the same
// relation more than once during the same call to transform.
// For example, access(_1, *), access(_1, _2).
//
// As the first approximation, marking this as undefined behavior and ignoring.

template <typename CandidatePayload, typename PatternPayload, typename... Args>
isl::union_map findAndReplace(isl::union_map umap,
                              Replacement<CandidatePayload, PatternPayload> arg,
                              Args... args);

} // namespace matchers

#include "access-inl.h"

#endif // ACCESS_H
