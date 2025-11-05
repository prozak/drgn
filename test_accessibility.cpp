// Test program for member accessibility feature
class TestClass {
private:
    int private_member;
    double private_double;

protected:
    float protected_member;
    long protected_long;

public:
    char public_member;
    short public_short;
};

struct TestStruct {
    int public_by_default;  // Should be public
    double another_public;  // Should be public
};

union TestUnion {
    int union_int;  // Should be public
    float union_float;  // Should be public
};

int main() {
    TestClass obj;
    TestStruct s;
    TestUnion u;
    return 0;
}
