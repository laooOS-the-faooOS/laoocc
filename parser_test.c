struct Foo {
    int x;
    int y;
};

int main(void)
{
    struct Foo foo;
    foo.x = 1;
    foo.y = 2;
    return foo.x + foo.y;
}
