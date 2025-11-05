# Member Accessibility Implementation Summary

## Overview
This document summarizes the implementation of member accessibility support in drgn, addressing [issue #83](https://github.com/osandov/drgn/issues/83).

## Implementation Status: COMPLETE ✓

The feature was fully implemented in commit 7d80b18.

## What Was Implemented

### 1. C API (`libdrgn/drgn.h`)
- Added `enum drgn_member_accessibility` with three values:
  - `DRGN_MEMBER_ACCESSIBILITY_PUBLIC = 1`
  - `DRGN_MEMBER_ACCESSIBILITY_PROTECTED = 2`
  - `DRGN_MEMBER_ACCESSIBILITY_PRIVATE = 3`
- Added `accessibility` field to `struct drgn_type_member`
- Values match DWARF 5 specification (DW_ACCESS_public/protected/private)

### 2. DWARF Parsing (`libdrgn/dwarf_info.c`)
- Modified `parse_member()` function to parse `DW_AT_accessibility` attribute
- Implements DWARF 5 spec section 5.7.6 defaults:
  - **Class members**: Default to **private** if DW_AT_accessibility is absent
  - **Struct/Union members**: Default to **public** if DW_AT_accessibility is absent
- Handles all three accessibility codes (public/protected/private)
- Provides error handling for invalid or unknown accessibility values

### 3. Internal API (`libdrgn/type.h` and `libdrgn/type.c`)
- Updated `drgn_compound_type_builder_add_member()` signature to accept accessibility parameter
- Modified implementation to store accessibility in `struct drgn_type_member`

### 4. Python Bindings (`libdrgn/python/`)
#### Python C Extension (`libdrgn/python/type.c`)
- Updated `TypeMember` structure to include `accessibility` field (`libdrgn/python/drgnpy.h`)
- Modified `TypeMember_new()` to accept optional `accessibility` parameter (defaults to public=1)
- Added `accessibility` to `TypeMember_members` array for direct attribute access
- Updated `TypeMember_dealloc()` to properly clean up accessibility reference
- Modified `unpack_member()` to extract accessibility when creating compound types

#### Python Type Stubs (`_drgn.pyi`)
- Added `accessibility: int` parameter to `TypeMember.__init__()`
- Added `accessibility: Final[int]` attribute to `TypeMember` class
- Documented accessibility values:
  - 1 = public
  - 2 = protected
  - 3 = private
- Documented default behavior per DWARF 5 spec

### 5. Pretty Printing (`libdrgn/language_c.c`)
- Updated `c_define_compound()` function to output accessibility labels
- For C++ **classes only**, prints accessibility labels when they change:
  - `public:`
  - `protected:`
  - `private:`
- Tracks previous accessibility to only print labels when changing sections
- Handles unknown accessibility values gracefully

## Technical Details

### Accessibility Values
The implementation uses integer values matching the DWARF 5 specification:
```c
enum drgn_member_accessibility {
    DRGN_MEMBER_ACCESSIBILITY_PUBLIC = 1,     // DW_ACCESS_public
    DRGN_MEMBER_ACCESSIBILITY_PROTECTED = 2,  // DW_ACCESS_protected
    DRGN_MEMBER_ACCESSIBILITY_PRIVATE = 3,    // DW_ACCESS_private
};
```

### Default Accessibility
Per DWARF 5 spec section 5.7.6:
- **struct** and **union**: Members are **public** by default
- **class**: Members are **private** by default

This matches C++ language semantics.

### Python Usage Example
```python
from drgn import Program, TypeMember

prog = Program()
int_type = prog.int_type("int", 4, True)

# Create class with mixed accessibility
class_type = prog.class_type(
    "MyClass",
    12,
    (
        TypeMember(int_type, "private_field", 0, accessibility=3),    # private
        TypeMember(int_type, "protected_field", 32, accessibility=2), # protected
        TypeMember(int_type, "public_field", 64, accessibility=1),    # public
    ),
)

# Access member accessibility
for member in class_type.members:
    print(f"{member.name}: accessibility={member.accessibility}")
```

### C Pretty Printing Example
For a C++ class, the output now includes accessibility labels:
```cpp
class MyClass {
private:
    int private_field;
protected:
    int protected_field;
public:
    int public_field;
}
```

## Testing

### Test Files Included
1. **test_accessibility.cpp** - Sample C++ program with various accessibility modifiers
   - Contains class with private/protected/public members
   - Contains struct with default public members
   - Contains union with default public members
   - Compiled with debug info for DWARF testing

2. **test_accessibility_feature.py** - Python unit tests for the feature
   - Tests default accessibility (public)
   - Tests explicit accessibility values
   - Tests struct with accessibility
   - Tests class with mixed accessibility
   - Verifies accessibility enum values

3. **test_member_accessibility.c** - C program for testing libdrgn API
   - Loads debug info from compiled binary
   - Verifies accessibility parsing from DWARF
   - Tests default accessibility for different type kinds
   - Validates member accessibility values

## Files Modified

The following files were modified to implement this feature:

- `libdrgn/drgn.h` - Added enum and struct field
- `libdrgn/dwarf_info.c` - Added DWARF parsing logic
- `libdrgn/type.h` - Updated function signature
- `libdrgn/type.c` - Updated function implementation
- `libdrgn/python/drgnpy.h` - Updated Python struct
- `libdrgn/python/type.c` - Updated Python bindings
- `libdrgn/language_c.c` - Added pretty printing
- `_drgn.pyi` - Updated type stubs

## Verification

The implementation has been verified to include:
- ✓ Enum definition with correct DWARF values
- ✓ Struct field addition
- ✓ DWARF attribute parsing
- ✓ Default accessibility logic per DWARF 5 spec
- ✓ Python bindings with parameter and attribute
- ✓ Type stub documentation
- ✓ Pretty printing for C++ classes
- ✓ Error handling for invalid values

## References

- **DWARF 5 Specification**:
  - Section 2.8: Accessibility Codes (DW_ACCESS_*)
  - Section 5.7.6: Data Member Entries (DW_AT_accessibility)
- **GitHub Issue**: https://github.com/osandov/drgn/issues/83
- **Commit**: 7d80b18

## Notes

- The accessibility attribute is only present in DWARF debug information for languages that support access control (like C++)
- For C struct/union types, all members will have public accessibility
- The feature maintains backward compatibility - existing code without accessibility will default to public (1)
