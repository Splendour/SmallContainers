#include <cassert>
#include <memory>

#include "../src/SmallVector.h"
#include "../src/SmallString.h"


#define TEST(expr) checkTest(expr, #expr, __LINE__)

void checkTest(bool _expr, const char* _exprStr, int _line)
{
    if (!_expr)
    {
        printf("Failed testcase on line %d: TEST(%s)\n", _line, _exprStr);
        fflush(stdout);
        throw std::exception();
    }
 }

struct NonPod
{
    NonPod(int val) { a = std::make_unique<int>(val); }
    ~NonPod() { }
    NonPod(NonPod&&) = default;

    std::unique_ptr<int> a;
};

struct Instrumented
{
    static int construct;
    static int destruct;
    static int assign;
    static int move;
    static int copy;

    Instrumented() { construct++; }
    ~Instrumented() { destruct++; }
    Instrumented(const Instrumented&) { construct++; copy++; }
    Instrumented(Instrumented&&) { construct++;  move++; }
    Instrumented& operator=(const Instrumented&) { assign++; copy++; return *this; };
    Instrumented& operator=(Instrumented&&) { assign++; move++; return *this; };

    static void reset()
    {
        construct = 0;
        destruct = 0;
        assign = 0;
        move = 0;
        copy = 0;
    }
};

int Instrumented::construct = 0;
int Instrumented::destruct = 0;
int Instrumented::assign = 0;
int Instrumented::move = 0;
int Instrumented::copy = 0;


void testVector()
{
    {
        sc::SmallVector<double, 32> v;
        TEST(v.capacity() == 32);
        TEST(v.size() == 0);
        TEST(v.empty());

        for (int i = 0; i < 33; ++i)
        {
            v.push_back(double(i));
        }

        for (int i = 0; i < 33; ++i)
        {
            TEST(v[i] == i);
        }

        TEST(v.size() == 33);
    }
    
    {
        sc::SmallVector<NonPod, 0> v;
        v.emplace_back(15);
        TEST(*v[0].a == 15);
    }

    {
        Instrumented::reset();
        sc::SmallVector<Instrumented, 1> v;
        Instrumented n;
        v.push_back(n);
        TEST(Instrumented::construct == 2);
        TEST(Instrumented::copy == 1);
        v.emplace_back(std::move(n)); // resize
        TEST(Instrumented::move == 2);
        TEST(Instrumented::construct == 4);
        TEST(Instrumented::destruct == 1);
        TEST(Instrumented::copy == 1);
        v.pop_back();
        v.pop_back();
        TEST(Instrumented::destruct == 3);
        v.emplace_back();
        v.emplace_back();
        TEST(Instrumented::construct == 6);
        TEST(Instrumented::move == 2);
        TEST(Instrumented::copy == 1);

        Instrumented cpy;
        Instrumented::reset();
        v[0] = cpy;
        v[1] = std::move(cpy);
        TEST(Instrumented::assign == 2);
        TEST(Instrumented::move == 1);
        TEST(Instrumented::copy == 1);
        v.resize(16);
        TEST(Instrumented::construct == 16);
    }
}

void testString()
{
    {
        sc::SmallString<32> s;
        TEST(s.empty());
        const char* t = "t";
        s = t;
        TEST(strcmp(s.c_str(), t) == 0);
        TEST(s.size() == strlen(t));
        std::string a = "tt";
        s.append(a);
        TEST(strcmp(s.c_str(), "ttt") == 0);
    }

    {
        std::string a = "237480374876238042380728";
        sc::SmallString<4> b(a);
        TEST(b.compare(a) == 0);
        auto sl = b.slice(1, 4);
        TEST(sl.compare("374") == 0);
        sl = b.substr(0, 0);
        TEST(sl.compare("") == 0);
    }
}

int main()
{
    testVector();
    testString();
    return 0;
}