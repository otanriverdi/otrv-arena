#ifndef OTRV_ARENA_H
#define OTRV_ARENA_H

#include <new>
#include <stddef.h>

struct Arena {
  size_t cursor = 0;
  size_t size;
  void *ptr;
};
void *arenaAlloc(Arena &arena, size_t size, size_t alignment);
void arenaReset(Arena &arena);

#ifdef OTRV_ARENA_IMPLEMENTATION

void *arenaAlloc(Arena &arena, size_t size, size_t alignment) {
  size_t misalignment = arena.cursor % alignment;
  if (misalignment != 0) {
    arena.cursor += alignment - misalignment;
  }

  if (arena.cursor + size > arena.size) {
    return nullptr;
  }
  void *start_of_block = static_cast<unsigned char *>(arena.ptr) + arena.cursor;
  arena.cursor += size;
  return start_of_block;
}

void arenaReset(Arena &arena) { arena.cursor = 0; }

#endif

template <typename T> struct ArenaAllocator {
  using value_type = T;

  template <typename U> struct rebind {
    using other = ArenaAllocator<U>;
  };

  Arena &arena;

  explicit ArenaAllocator(Arena &a) noexcept : arena(a) {};
  template <typename U>
  ArenaAllocator(const ArenaAllocator<U> &other) noexcept
      : arena(other.arena) {}

  T *allocate(size_t n) {
    void *mem = arenaAlloc(arena, n * sizeof(T), alignof(T));
    if (!mem) {
      throw std::bad_alloc();
    }
    return static_cast<T *>(mem);
  };

  void deallocate(T *ptr, size_t n) {
    // No-op
  }

  template <typename U> friend struct ArenaAllocator;
};

template <typename T, typename U>
bool operator==(const ArenaAllocator<T> &lhs,
                const ArenaAllocator<U> &rhs) noexcept {
  return &lhs.arena == &rhs.arena;
}

template <typename T, typename U>
bool operator!=(const ArenaAllocator<T> &lhs,
                const ArenaAllocator<U> &rhs) noexcept {
  return !(lhs == rhs);
}

#endif
