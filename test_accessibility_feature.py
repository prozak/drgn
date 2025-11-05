#!/usr/bin/env python3
"""
Test script for member accessibility feature.
This verifies that TypeMember.accessibility is properly exposed and working.
"""

import sys
import unittest
from drgn import Program, TypeMember


class TestMemberAccessibility(unittest.TestCase):
    def setUp(self):
        self.prog = Program()

    def test_typemember_default_accessibility(self):
        """Test that TypeMember defaults to public accessibility (1)."""
        int_type = self.prog.int_type("int", 4, True)
        member = TypeMember(int_type, "test_member", 0)
        self.assertEqual(member.accessibility, 1, "Default accessibility should be public (1)")

    def test_typemember_explicit_accessibility(self):
        """Test that TypeMember accepts explicit accessibility values."""
        int_type = self.prog.int_type("int", 4, True)

        # Test public (1)
        member_public = TypeMember(int_type, "public_member", 0, accessibility=1)
        self.assertEqual(member_public.accessibility, 1, "Public accessibility should be 1")

        # Test protected (2)
        member_protected = TypeMember(int_type, "protected_member", 32, accessibility=2)
        self.assertEqual(member_protected.accessibility, 2, "Protected accessibility should be 2")

        # Test private (3)
        member_private = TypeMember(int_type, "private_member", 64, accessibility=3)
        self.assertEqual(member_private.accessibility, 3, "Private accessibility should be 3")

    def test_struct_with_accessibility(self):
        """Test creating a struct with members having different accessibility."""
        int_type = self.prog.int_type("int", 4, True)
        double_type = self.prog.float_type("double", 8)

        struct = self.prog.struct_type(
            "TestStruct",
            16,
            (
                TypeMember(int_type, "public_field", 0, accessibility=1),
                TypeMember(double_type, "another_public", 64, accessibility=1),
            ),
        )

        self.assertEqual(len(struct.members), 2)
        self.assertEqual(struct.members[0].name, "public_field")
        self.assertEqual(struct.members[0].accessibility, 1)
        self.assertEqual(struct.members[1].name, "another_public")
        self.assertEqual(struct.members[1].accessibility, 1)

    def test_class_with_mixed_accessibility(self):
        """Test creating a class with mixed public/protected/private members."""
        int_type = self.prog.int_type("int", 4, True)
        double_type = self.prog.float_type("double", 8)
        float_type = self.prog.float_type("float", 4)

        class_type = self.prog.class_type(
            "TestClass",
            24,
            (
                TypeMember(int_type, "private_member", 0, accessibility=3),
                TypeMember(double_type, "private_double", 32, accessibility=3),
                TypeMember(float_type, "protected_float", 96, accessibility=2),
                TypeMember(int_type, "public_int", 128, accessibility=1),
            ),
        )

        self.assertEqual(len(class_type.members), 4)
        self.assertEqual(class_type.members[0].accessibility, 3)  # private
        self.assertEqual(class_type.members[1].accessibility, 3)  # private
        self.assertEqual(class_type.members[2].accessibility, 2)  # protected
        self.assertEqual(class_type.members[3].accessibility, 1)  # public

    def test_accessibility_values(self):
        """Verify the accessibility enum values match the specification."""
        # Per DWARF spec and drgn implementation:
        # 1 = public (DW_ACCESS_public)
        # 2 = protected (DW_ACCESS_protected)
        # 3 = private (DW_ACCESS_private)
        int_type = self.prog.int_type("int", 4, True)

        public_member = TypeMember(int_type, "pub", 0, accessibility=1)
        self.assertEqual(public_member.accessibility, 1)

        protected_member = TypeMember(int_type, "prot", 0, accessibility=2)
        self.assertEqual(protected_member.accessibility, 2)

        private_member = TypeMember(int_type, "priv", 0, accessibility=3)
        self.assertEqual(private_member.accessibility, 3)


if __name__ == "__main__":
    # Run tests
    loader = unittest.TestLoader()
    suite = loader.loadTestsFromTestCase(TestMemberAccessibility)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)

    # Exit with appropriate code
    sys.exit(0 if result.wasSuccessful() else 1)
