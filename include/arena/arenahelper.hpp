/*
 * Copyright (c) 2024 ClapDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory_resource>
#include <type_traits>
#include <utility>

// NOLINTBEGIN
#include "arena/align.hpp"
#define ArenaFullManagedTag using ArenaManaged_ = void; // NOLINT
#define ArenaManagedCreateOnlyTag                                              \
  using ArenaManagedSkipDestruct_ = void; // NOLINT

// NOLINTEND

namespace arena {

/*
 * Detect if T is a PMR container with trivially destructible value_type.
 * For such containers, we can skip destructor registration because:
 * 1. Arena's deallocate() is a no-op
 * 2. The elements don't need destruction
 */
template <typename T, typename = void>
struct is_pmr_trivial_container : std::false_type {};

template <typename T>
struct is_pmr_trivial_container<
    T,
    std::void_t<typename T::allocator_type, typename T::value_type,
                std::enable_if_t<
                    std::is_same_v<typename T::allocator_type,
                                   std::pmr::polymorphic_allocator<
                                       typename T::value_type>> &&
                    std::is_trivially_destructible_v<typename T::value_type>>>>
    : std::true_type {};

/*
 * class ArenaHelper is for helping the Type stored in the Arena memory.
 *
 * it uses type_traits and constexpr techniques to indicates supporting for
 * arena for a type T at compiler time.
 */
class Arena;

template <typename T> class ArenaHelper {
public:
  template <typename U>
  static auto
  DestructionSkippable(const typename U::ArenaManagedSkipDestruct_ *) -> char;

  template <typename U> static auto DestructionSkippable(...) -> double;

  // Skip destructor registration if:
  // 1. Has ArenaManagedSkipDestruct_ tag, OR
  // 2. Is trivially destructible, OR
  // 3. Is a PMR container with trivially destructible elements
  using is_destructor_skippable =
      std::integral_constant<bool,
                             sizeof(DestructionSkippable<T>(
                                 static_cast<const T *>(0))) == sizeof(char) ||
                                 std::is_trivially_destructible<T>::value ||
                                 is_pmr_trivial_container<T>::value>;

  template <typename U>
  static auto ArenaConstructable(const typename U::ArenaManaged_ *) -> char;
  template <typename U> static auto ArenaConstructable(...) -> double;

  using is_arena_constructable =
      std::integral_constant<bool,
                             sizeof(ArenaConstructable<T>(
                                 static_cast<const T *>(0))) == sizeof(char)>;

  friend class Arena;
};

/*
 * is_arena_full_managable<T>::value is true if the message type T has arena
 * support enabled, and false otherwise.
 *
 * is_destructor_skippable<T>::value is true if the message type T has told
 * the arena that it is safe to skip the destructor, and false otherwise.
 *
 * This is inside Arena because only Arena has the friend relationships
 * necessary to see the underlying generated code traits.
 */
template <typename T>
struct is_arena_full_managable : ArenaHelper<T>::is_arena_constructable {};
template <typename T>
struct is_destructor_skippable : ArenaHelper<T>::is_destructor_skippable {};

template <typename T>
concept Constructable =
    is_arena_full_managable<T>::value || is_destructor_skippable<T>::value;

template <typename T>
concept NonConstructable = !is_arena_full_managable<T>::value;

template <typename T>
concept DestructorSkippable = is_destructor_skippable<T>::value;

} // namespace arena
