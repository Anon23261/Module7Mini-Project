# Problem Set 1: Memory Allocator Implementation

## Overview
In this problem set, you will implement and analyze various memory allocation strategies. You'll work with both the Buddy Allocator and Slab Allocator, comparing their performance characteristics and understanding their trade-offs.

## Objectives
- Implement core memory allocation algorithms
- Analyze allocator performance under different workloads
- Design thread-safe memory management solutions
- Debug memory-related issues

## Requirements

### Part 1: Buddy Allocator Enhancement (40 points)
Enhance the provided Buddy Allocator implementation with the following features:

1. **Coalescing Optimization (15 points)**
   - Implement smart block merging
   - Add split prediction
   - Minimize fragmentation

2. **Memory Tracking (10 points)**
   - Track allocation patterns
   - Generate usage statistics
   - Implement memory leak detection

3. **Performance Optimization (15 points)**
   - Optimize block search
   - Implement caching strategies
   - Reduce allocation overhead

### Part 2: Slab Allocator Analysis (40 points)
Analyze and improve the Slab Allocator implementation:

1. **Cache Analysis (15 points)**
   - Measure cache hit rates
   - Analyze memory access patterns
   - Implement cache coloring

2. **Thread Safety (15 points)**
   - Add fine-grained locking
   - Implement lock-free operations
   - Handle concurrent allocations

3. **Error Handling (10 points)**
   - Implement robust error checking
   - Add debugging facilities
   - Create error recovery mechanisms

### Part 3: Comparative Analysis (20 points)
Write a detailed analysis comparing the two allocators:

1. **Performance Comparison (10 points)**
   - Benchmark both allocators
   - Analyze memory usage
   - Compare throughput and latency

2. **Use Case Analysis (10 points)**
   - Identify optimal use cases
   - Document trade-offs
   - Provide implementation recommendations

## Submission Requirements

### Code Submission
- Submit all source files via Git
- Include comprehensive unit tests
- Provide build instructions

### Documentation
- Technical design document
- Performance analysis report
- API documentation

### Analysis Report
- Implementation decisions
- Performance metrics
- Trade-off analysis

## Grading Criteria

### Implementation (60%)
- Correctness (20%)
- Efficiency (20%)
- Code quality (20%)

### Analysis (30%)
- Depth of analysis (15%)
- Supporting data (15%)

### Documentation (10%)
- Clarity (5%)
- Completeness (5%)

## Tips for Success
1. Start with basic implementations
2. Add features incrementally
3. Test thoroughly at each step
4. Document design decisions
5. Profile before optimizing

## Resources
- Course lecture notes
- Memory Management textbook
- Online debugging tools
- Performance profiling guides

## Academic Integrity
This is an individual assignment. While you may discuss concepts with classmates, all code and analysis must be your own work.

## Submission Deadline
Due: Friday, Week 2, 11:59 PM EST

## Extension Policy
- One-day extension: -10%
- Two-day extension: -25%
- No submissions accepted after two days

## Questions
Post questions on Piazza or attend office hours:
- Professor hours: Tuesday/Thursday 2-4 PM
- TA hours: Monday/Wednesday/Friday 1-3 PM
