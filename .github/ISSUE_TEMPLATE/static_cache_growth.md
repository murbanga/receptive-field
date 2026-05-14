---
name: Static Cache Growth Issue
about: Track the unbounded growth of the attribute_edit_cache static map
title: "Optimize: Unbounded static cache growth in main.cpp"
labels: optimization, memory
---

## Issue: Static Cache Growth

### Description
The `attribute_edit_cache` static map in `main.cpp` (line 281) grows unbounded during application runtime without any mechanism to limit its size or clear stale entries.

### Location
**File**: `receptive_field_view/main.cpp`
**Line**: 281
```cpp
static std::map<std::string, std::string> attribute_edit_cache;
```

### Impact
- While not a critical memory leak, the cache accumulates entries indefinitely
- Memory grows proportionally with the number of unique tensor attributes accessed during a session
- Could become problematic in long-running applications or when working with many different models

### Current Behavior
Every time an attribute is accessed in the UI, a new key-value pair is added to the cache if it doesn't exist:
```cpp
for (auto &a : it_node->second.attrs) {
    std::string key = selected.name + "_" + a.first;
    if (attribute_edit_cache.find(key) == attribute_edit_cache.end()) {
        attribute_edit_cache[key] = attribute_to_string(a.second);  // ← Keeps growing
    }
    InputText(a.first.c_str(), &attribute_edit_cache.at(key));
}
```

### Suggested Solutions

1. **Add Maximum Size Limit** - Implement an LRU (Least Recently Used) eviction policy
2. **Clear on Graph Reload** - Reset the cache when a new graph is loaded
3. **Memory Pool** - Use a fixed-size memory pool or object pool pattern
4. **Scope Reduction** - Consider if the cache is truly necessary or if temporary storage would suffice

### Priority
🟢 **Low** - Not a critical leak, but good to address for long-running sessions

### Acceptance Criteria
- [ ] Static cache has a maximum size limit OR is cleared on appropriate events
- [ ] No memory growth after extended usage with multiple models
- [ ] Performance remains unaffected or improves
