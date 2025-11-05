// Test program to verify member accessibility parsing
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libdrgn/drgn.h"

static const char *accessibility_to_string(enum drgn_member_accessibility acc) {
    switch (acc) {
    case DRGN_MEMBER_ACCESSIBILITY_PUBLIC:
        return "public";
    case DRGN_MEMBER_ACCESSIBILITY_PROTECTED:
        return "protected";
    case DRGN_MEMBER_ACCESSIBILITY_PRIVATE:
        return "private";
    default:
        return "unknown";
    }
}

static int test_type_accessibility(struct drgn_program *prog, const char *type_name,
                                    const char *expected[][2], size_t num_expected) {
    struct drgn_error *err;
    struct drgn_type *type;

    err = drgn_program_find_type(prog, type_name, NULL, &type);
    if (err) {
        fprintf(stderr, "Failed to find type %s: %s\n", type_name, err->message);
        drgn_error_destroy(err);
        return 1;
    }

    struct drgn_type_member *members = drgn_type_members(type);
    size_t num_members = drgn_type_num_members(type);

    printf("\nTesting %s (found %zu members):\n", type_name, num_members);

    if (num_members != num_expected) {
        fprintf(stderr, "ERROR: Expected %zu members, got %zu\n", num_expected, num_members);
        return 1;
    }

    int errors = 0;
    for (size_t i = 0; i < num_members; i++) {
        const char *member_name = members[i].name ? members[i].name : "<unnamed>";
        const char *access_str = accessibility_to_string(members[i].accessibility);

        printf("  Member %zu: %s - accessibility: %s\n", i, member_name, access_str);

        if (expected[i][0] && strcmp(member_name, expected[i][0]) != 0) {
            fprintf(stderr, "    ERROR: Expected name '%s', got '%s'\n",
                    expected[i][0], member_name);
            errors++;
        }

        if (expected[i][1] && strcmp(access_str, expected[i][1]) != 0) {
            fprintf(stderr, "    ERROR: Expected accessibility '%s', got '%s'\n",
                    expected[i][1], access_str);
            errors++;
        }
    }

    return errors;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path-to-test-binary>\n", argv[0]);
        return 1;
    }

    struct drgn_program *prog;
    struct drgn_error *err;

    err = drgn_program_create(NULL, &prog);
    if (err) {
        fprintf(stderr, "Failed to create program: %s\n", err->message);
        drgn_error_destroy(err);
        return 1;
    }

    err = drgn_program_load_debug_info(prog, (const char **)&argv[1], 1,
                                       false, false);
    if (err) {
        fprintf(stderr, "Failed to load debug info: %s\n", err->message);
        drgn_error_destroy(err);
        drgn_program_destroy(prog);
        return 1;
    }

    int total_errors = 0;

    // Test class with explicit accessibility
    const char *class_expected[][2] = {
        {"private_member", "private"},
        {"private_double", "private"},
        {"protected_member", "protected"},
        {"protected_long", "protected"},
        {"public_member", "public"},
        {"public_short", "public"},
    };
    total_errors += test_type_accessibility(prog, "TestClass", class_expected, 6);

    // Test struct (should default to public)
    const char *struct_expected[][2] = {
        {"public_by_default", "public"},
        {"another_public", "public"},
    };
    total_errors += test_type_accessibility(prog, "TestStruct", struct_expected, 2);

    // Test union (should default to public)
    const char *union_expected[][2] = {
        {"union_int", "public"},
        {"union_float", "public"},
    };
    total_errors += test_type_accessibility(prog, "TestUnion", union_expected, 2);

    drgn_program_destroy(prog);

    if (total_errors == 0) {
        printf("\n✓ All tests passed!\n");
        return 0;
    } else {
        printf("\n✗ %d errors found\n", total_errors);
        return 1;
    }
}
