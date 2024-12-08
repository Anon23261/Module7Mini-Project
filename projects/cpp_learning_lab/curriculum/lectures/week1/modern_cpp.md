# Modern C++ Programming: Memory Model and Resource Management

## Lecture 1: Memory Model and RAII

### 1. C++ Memory Model

#### 1.1 Memory Regions
- Stack
  - Automatic storage duration
  - LIFO structure
  - Fast allocation/deallocation
  - Size limitations
  
- Heap
  - Dynamic storage duration
  - Manual management (with smart pointers)
  - Flexible size
  - Fragmentation concerns

- Static/Global
  - Program lifetime duration
  - Initialized before main()
  - Potential initialization order issues

#### 1.2 Memory Ordering
```cpp
std::atomic<int> x{0};
std::atomic<int> y{0};

// Thread 1
x.store(1, std::memory_order_release);

// Thread 2
if (x.load(std::memory_order_acquire) == 1) {
    // Guaranteed to see Thread 1's writes
}
```

### 2. RAII (Resource Acquisition Is Initialization)

#### 2.1 Core Principles
- Resource management tied to object lifetime
- Exception safety
- Deterministic cleanup
- Scope-based management

#### 2.2 Implementation Example
```cpp
template<typename T>
class UniqueResource {
    T* ptr_;
public:
    explicit UniqueResource(T* p) noexcept : ptr_(p) {}
    ~UniqueResource() { delete ptr_; }
    
    // Prevent copying
    UniqueResource(const UniqueResource&) = delete;
    UniqueResource& operator=(const UniqueResource&) = delete;
    
    // Allow moving
    UniqueResource(UniqueResource&& other) noexcept 
        : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }
    
    UniqueResource& operator=(UniqueResource&& other) noexcept {
        if (this != &other) {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }
};
```

### 3. Smart Pointers

#### 3.1 std::unique_ptr
- Single ownership
- Zero overhead
- Move-only semantics
- Custom deleters

```cpp
auto createResource() {
    return std::make_unique<Resource>();
}

void useResource(std::unique_ptr<Resource> res) {
    // Resource automatically cleaned up
}
```

#### 3.2 std::shared_ptr
- Shared ownership
- Reference counting
- Thread-safe reference count
- Potential circular references

```cpp
struct Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;  // Break cycles
};
```

### 4. Memory Safety

#### 4.1 Undefined Behavior
- Dangling references
- Buffer overflows
- Use after free
- Double deletion

#### 4.2 Prevention Techniques
- RAII
- Smart pointers
- Bounds checking
- Static analysis
- Sanitizers

### 5. Modern Best Practices

#### 5.1 Guidelines
- Prefer stack to heap
- Use smart pointers over raw pointers
- Implement move semantics
- Follow the Rule of Zero/Five
- Use std::array over C-style arrays

#### 5.2 Performance Considerations
- Move semantics optimization
- Small string optimization
- Return value optimization (RVO)
- Copy elision

## Exercises

1. Implement a thread-safe resource pool using RAII
2. Create a custom smart pointer with logging capabilities
3. Design a memory-leak detector using custom allocators
4. Implement a circular buffer with proper memory management

## Reading Materials

- Effective Modern C++ by Scott Meyers
- C++ Core Guidelines
- C++ Reference: Memory Model

## Next Lecture
- Templates and Template Metaprogramming
- STL Containers and Algorithms
- Custom Allocators
