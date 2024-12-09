# Final Project: Advanced Memory Management System

## Overview
Design and implement a comprehensive memory management system that combines multiple allocation strategies. Your system should adaptively choose the most appropriate allocator based on allocation patterns and requirements.

## Project Goals
1. Create a unified memory management interface
2. Implement multiple allocation strategies
3. Design an intelligent strategy selector
4. Provide comprehensive monitoring and debugging tools

## Requirements

### Core Components (50%)

#### 1. Unified Allocator Interface (15%)
- Design a flexible allocator interface
- Support multiple allocation strategies
- Implement strategy switching mechanism

#### 2. Allocation Strategies (20%)
Implement at least three of the following:
- Buddy Allocator
- Slab Allocator
- Pool Allocator
- Sequential Fit Allocator
- Custom allocation strategy

#### 3. Strategy Selector (15%)
- Implement allocation pattern analysis
- Create strategy selection heuristics
- Support runtime strategy switching

### Advanced Features (30%)

#### 1. Performance Optimization (10%)
- Cache-aware allocation
- Thread-safe operations
- Memory alignment handling

#### 2. Monitoring System (10%)
- Memory usage tracking
- Performance metrics collection
- Allocation pattern analysis

#### 3. Debug Support (10%)
- Memory leak detection
- Corruption detection
- Detailed error reporting

### Documentation and Analysis (20%)

#### 1. Technical Documentation (10%)
- Architecture overview
- API documentation
- Implementation details

#### 2. Performance Analysis (10%)
- Benchmark results
- Strategy comparison
- Use case recommendations

## Deliverables

### 1. Source Code
- Complete implementation
- Unit tests
- Integration tests
- Build system configuration

### 2. Documentation
- Design document
- API reference
- User guide
- Performance report

### 3. Presentation
- System demonstration
- Performance analysis
- Design decisions
- Lessons learned

## Timeline

### Week 1: Design Phase
- Architecture design
- Interface definition
- Strategy planning

### Week 2: Implementation
- Core components
- Basic strategies
- Initial testing

### Week 3: Advanced Features
- Optimization
- Monitoring
- Debugging tools

### Week 4: Integration and Testing
- System integration
- Performance testing
- Documentation
- Final presentation

## Evaluation Criteria

### Implementation (50%)
- Correctness (20%)
- Performance (15%)
- Code quality (15%)

### Features (30%)
- Functionality (15%)
- Innovation (15%)

### Documentation (20%)
- Completeness (10%)
- Clarity (10%)

## Submission Guidelines

### Code Repository
- Clean, documented code
- Comprehensive test suite
- Build instructions

### Documentation Package
- Design documents
- API reference
- Performance analysis
- User guide

### Presentation Materials
- Slides
- Demo script
- Benchmark results

## Resources
- Course materials
- Research papers
- Technical documentation
- Profiling tools

## Academic Integrity
- Original work required
- Proper citation needed
- Collaboration policy applies

## Support
- Weekly check-ins
- Office hours
- Online support
- Code reviews

## Grading Rubric

### Technical Achievement (40%)
- Implementation completeness
- Performance optimization
- Feature innovation

### Engineering Practice (30%)
- Code quality
- Testing coverage
- Documentation

### Project Management (30%)
- Timeline adherence
- Progress communication
- Final presentation

## Extension Opportunities
- Custom allocator design
- Advanced optimization techniques
- Integration with real-world systems

## Final Notes
This project is an opportunity to demonstrate your understanding of memory management concepts and your ability to implement them in a practical system. Focus on creating a robust, well-documented solution that could be used in real-world applications.

## New Methods

### SlabAllocator
- `size_t get_num_slabs() const noexcept`: Returns the number of slabs in the allocator.
- `size_t get_num_free_objects() const noexcept`: Returns the number of free objects in the allocator.

### PoolAllocator
- `size_t get_num_free_blocks() const noexcept`: Returns the number of free blocks in the allocator.

### BuddyAllocator
- `size_t get_num_free_blocks() const noexcept`: Returns the number of free blocks in the allocator.
