# Arena - A C++20 Memory Allocation Library

Arena is a high-performance memory allocation library inspired by Google Chrome project, providing efficient memory management with improved data locality and automatic cleanup.

## Purpose

Arena is designed to align heap memory layout with OS/hardware memory paging functions, providing several key benefits:

- **Better Data Locality**: More gathered heap layout improves cache performance
- **CPU-Optimized Alignment**: All memory blocks are aligned to CPU word size, preventing cache line pollution  
- **Simplified Memory Management**: No need to call delete/free functions - Arena destroys entire memory blocks in a single operation
- **Object Lifecycle Management**: Associated objects are destroyed at a single point
- **STL Container Optimization**: With C++20 power, standard containers can use Arena to optimize memory layout

> **Note**: C++ program's standard containers often have poor memory locality because STL's default allocator creates scattered memory layouts.

## Trade-offs

Arena uses padding to optimize CPU cache line usage, which results in higher memory overhead but significantly better performance for allocation-heavy workloads.

## Requirements

- **C++20 Standard**: Full C++20 support required
- **std::source_location**: Required (note: Apple Clang may not support this yet)

## Compatibility

Arena works seamlessly with:
- **tcmalloc**, **jemalloc**, and other malloc implementations
- **[Seastar](https://github.com/scylladb/seastar)** for shared-nothing architecture

## Architecture

### Memory Layout

An Arena maintains a chain of memory blocks, where each block is a linear continuous memory area:

```
                           +-------+                         +-------+
          +------+         |Cleanup|      +------+           |Cleanup|
          |Header|         | Area  |      |Header|           | Area  |
          +------+         +-------+      +------+           +-------+
          +------+-----------+---+        +-----+------------+-------+
          |      |           |   |        |     |            |       |
          |      |           |   |        |     |            |       |
          |      |           |   |        <-----+ limit_ ---->       |
       +->|      |           |   |        |     |            |       |
       |  |      |           |   |        |     |            |       |
       |  |      |           |   |        <-----+---- size_ -+------->
       |  |      |           |   |        |     |            |       |
       |  +---+--+-----------+---+        +--+--+------------+-------+
       |      |                              |
       |      |                              |  +------+
<------+------+                              |  | Prev |
       |  +----------------+                 |  +------+
       |  | Prev = nullptr |                 |
       |  +----------------+                 |
       |                                     |
       |                                     |
       +-------------------------------------+
```

### Memory Alignment Benefits

Aligned memory allows CPUs to work in optimal conditions. Modern CPUs have complex memory and cache mechanisms designed specifically for aligned memory access patterns.

### Cleanup Area and Functions

The cleanup area stores destructor information for proper object lifecycle management. When the arena is destroyed or reset, all registered cleanup functions are called automatically.

## Type Requirements and Tags

Arena defines two important tags for type management:

1. **ArenaFullManagedTag** - Indicates the class can be created and managed by Arena
2. **ArenaManagedCreateOnlyTag** - Indicates Arena should skip calling the destructor

### Type Creation Rules

A class can be created in Arena if it meets ANY of these conditions:

1. **Simple C-like structs**: `is_trivial && is_standard_layout`
2. **Tagged classes**: Contains `ArenaFullManagedTag` or `ArenaManagedCreateOnlyTag`

#### Requirements Explained:
- **is_trivial**: Default constructor and copy constructor must be compiler-generated
- **is_standard_layout**: Simple memory layout with consistent access control, no virtual functions, no non-static reference members, no multiple inheritance

## Usage Examples

### 1. Pure C-like Structs

Simple structs that meet trivial and standard layout requirements:

```cpp
/*
 * Simplest struct - no tags needed
 * Can be Created and CreateArray
 */
struct simple_struct {
    int value;
    char flag;
};

Arena arena;
auto* obj = arena.Create<simple_struct>();
auto* array = arena.CreateArray<simple_struct>(100);
```

### 2. Inheritance with Standard Layout

```cpp
struct base {
    int base_value;
};

/*
 * Derived class maintaining standard_layout
 * Can be Created and CreateArray
 */
class derived_standard : public base {
   public:
    int derived_value;
};

Arena arena;
auto* obj = arena.Create<derived_standard>();
```

### 3. Non-Trivial Classes (Requires Tags)

```cpp
/*
 * Not trivial due to custom constructor
 * Cannot be Created without proper tag
 */
class complex_class {
   public:
    complex_class(int value) : data(value) {}
    ArenaFullManagedTag;  // Required for Arena creation
   private:
    int data;
};

Arena arena;
auto* obj = arena.Create<complex_class>(42);
```

### 4. Classes with Reference Members

```cpp
/*
 * Not standard_layout due to reference member
 * Requires ArenaFullManagedTag to be Created in Arena
 */
class arena_aware_class {
   public:
    arena_aware_class(Arena& a) : arena_ref(a) {}
    ArenaFullManagedTag;
   private:
    Arena& arena_ref;
};

Arena arena;
auto* obj = arena.Create<arena_aware_class>();  // Arena ref passed automatically
```

### 5. Container Integration

Arena integrates seamlessly with C++17 PMR (Polymorphic Memory Resources):

```cpp
Arena arena;

// Create PMR containers using Arena's memory resource
auto* vec = arena.Create<std::pmr::vector<int>>();
auto* str = arena.Create<std::pmr::string>();
auto* map = arena.Create<std::pmr::unordered_map<int, std::pmr::string>>();

// All allocations from these containers will use Arena memory
vec->push_back(42);
*str = "Hello Arena!";
(*map)[1] = "First entry";
```

### 6. Custom Container Classes

```cpp
class arena_vector {
   public:
    arena_vector(Arena& a) : arena_ref(a), data(a.get_memory_resource()) {}
    ArenaManagedCreateOnlyTag;  // Skip destructor - Arena handles cleanup
   private:
    Arena& arena_ref;
    std::pmr::vector<int> data;
};

Arena arena;
auto* container = arena.Create<arena_vector>();
```

### 7. Classes with Internal Pointers

```cpp
class pointer_holder {
   public:
    pointer_holder(some_type* ptr) : internal_ptr(ptr) {}
    ArenaFullManagedTag;
   private:
    some_type* internal_ptr;  // Must outlive the Arena
};

Arena arena;
some_type* long_lived_object = get_long_lived_object();
auto* holder = arena.Create<pointer_holder>(long_lived_object);
```

### 8. Cross-Arena References

```cpp
class multi_arena_class {
   public:
    multi_arena_class(Arena* other) : other_arena(other) {}
    ArenaFullManagedTag;
   private:
    Arena* other_arena;  // Can reference different Arena
};

Arena arena1, arena2;
auto* obj = arena1.Create<multi_arena_class>(&arena2);
```

## Best Practices

### Constructor Patterns

1. **Arena Reference as First Parameter**: When a class constructor needs an Arena reference, make it the first parameter. `Create()` will automatically pass `*this`:

```cpp
class arena_class {
    arena_class(Arena& arena) : arena_ref(arena) {}
    ArenaFullManagedTag;
private:
    Arena& arena_ref;
};

Arena arena;
auto* obj = arena.Create<arena_class>();  // Arena ref passed automatically
```

2. **Additional Parameters**: Pass other parameters normally:

```cpp
class parameterized_class {
    parameterized_class(Arena& arena, int value, const std::string& name) 
        : arena_ref(arena), value_(value), name_(name) {}
    ArenaFullManagedTag;
private:
    Arena& arena_ref;
    int value_;
    std::string name_;
};

Arena arena;
auto* obj = arena.Create<parameterized_class>(42, "example");
```

### Lifecycle Management Rules

1. **Single Ownership**: An instance should either be completely Arena-managed or not at all
2. **Cross-Arena Copying**: If copying between Arenas, ensure proper resource management
3. **Same-Arena Moving**: Move operations should only occur within the same Arena
4. **Cleanup Consistency**: Use `ArenaManagedCreateOnlyTag` when Arena handles all cleanup

## Advanced Features

### Memory Resource Integration

```cpp
void advanced_pmr_usage(Arena& arena) {
    // Create containers directly in Arena memory
    auto* str_vec = arena.Create<std::pmr::vector<std::pmr::string>>();
    
    // Create string directly in vector (no copy)
    str_vec->emplace_back("direct_construction_in_arena");
    
    // This involves copying
    std::pmr::string temp_str("temporary", arena.get_memory_resource());
    str_vec->push_back(temp_str);
}
```

### Performance Monitoring

Arena includes built-in metrics for monitoring allocation patterns and performance:

```cpp
Arena arena;
// ... use arena for allocations ...

// Get allocation statistics
auto stats = arena.get_metrics();
std::cout << "Allocations: " << stats.allocation_count << std::endl;
std::cout << "Memory used: " << stats.bytes_allocated << std::endl;
```

## Error Handling

Arena provides clear error conditions:

- **Allocation Failure**: Returns `nullptr` when memory cannot be allocated
- **Type Constraint Violations**: Compile-time errors for unsupported types
- **Lifecycle Violations**: Runtime assertions for improper usage patterns

## Thread Safety

Arena instances are **not thread-safe** by default. For multi-threaded usage:

1. Use separate Arena instances per thread
2. Implement external synchronization for shared Arenas
3. Consider thread-local Arena instances for best performance

## Performance Characteristics

- **Allocation**: O(1) amortized
- **Deallocation**: O(1) for entire Arena, individual objects cannot be freed
- **Memory Overhead**: Higher due to alignment padding
- **Cache Performance**: Significantly better due to improved locality

## Migration Guide

When migrating from standard allocators:

1. Replace `new/delete` with `arena.Create<T>()`
2. Replace `malloc/free` with `arena.AllocateAligned()`
3. Update containers to use `std::pmr` variants with Arena's memory resource
4. Add appropriate tags to custom classes
5. Ensure proper Arena lifetime management

## Future Improvements

1. Enhanced C++20 syntax modernization
2. Comprehensive API documentation generation
3. Additional performance optimization hooks
4. Extended STL container integration