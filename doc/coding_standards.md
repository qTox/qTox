# Coding Standards & Guidelines

This document defines the qTox coding standards and style, all code
contributions are expected to adhere to the rules described below.

Most stylistic features described below are described as clang-format rules
present in the root of the repository, as such most code formatting rules can
be applied by simply running clang-format over the source code in question.

You can run [`tools/format-code.sh`] to format all C++ files tracked by
git.

In case something is not specified in the following paragraphs, the
[CppCoreGuidelines] apply.

## Coding Standard

qTox is written under **[ISO/IEC 14882:2011 (C++11)][ISO/IEC/C++11]** without
GNU/GCC specific extensions (i.e. qTox should compile with `CXXFLAGS` set to
`-std=c++11`, regardless of if `-std=gnu+11` is being used during compile
time). Source code must be able to be compiled under multiple different
compilers and operating systems including but not limited to GCC and Clang on
Microsoft Windows, Apple OS X and GNU/Linux-based derivatives.

In addition to the base language, the following additional restrictions are
imposed:

### Compatibility

qTox is linked against Qt 5, allowing the use of Qt constructs and library
features. The current minimum supported Qt version is Qt 5.5, meaning that all
code must compile in a Qt 5.5 environment. Any usage post-Qt 5.5 features must
be optional and be disabled when compiling/running in a Qt 5.5 environment.

### No Exceptions

qTox is compiled without support for [C++11 exceptions][Exceptions], meaning
that any code contribution or dependency cannot throw a C++ exception at
runtime or else the application will crash. For code present in the qTox
repository, this is enforced by the use of the `-fno-exceptions` flag in the
CMake configuration.

Note: This restriction prohibits the use of external libraries that may throw
unhandled exceptions to qTox code. External libraries using exceptions, but
never require qTox code to handle them directly, will work fine.

### No RTTI

qTox is compiled without support for [RTTI], as such code contributions using
`dynamic_cast()` or `std::dynamic_pointer_cast()` may fail to compile and may
be rejected on this basis. The implications of this are that the signature of
all polymorphic types must be known at compile time or stored in an
implementation-specific way. In essence, if a substitution from
`dynamic_cast()` to `static_cast()` can be performed without affecting program
correctness, the construct in question is valid.

**Note: no usage of `dynamic_cast()` or `std::dynamic_pointer_cast()` is
permitted, even if the code compiles**. An optimizing compiler may be silently
replacing your dynamic casts with static casts if it can ensure the replacement
is to the same effect.

For manipulation of Qt-based objects, use `qobject_cast()` instead.

## Coding Style

### Indentation

All code is to be formatted with an indentation size of 4 characters, using
spaces. **Tabs are not permitted.** Scope specifiers and namespaces are not to
be indented.

The following example demonstrates well formatted code under the indentation
rules:

```c++
namespace Foo
{
class Bar
{
public:
    Bar()
    {
        // Some code here

        switch (...) {
        case 0: {
            // Some code here
        }
        }

        // More code

        if (...) {
            // Conditional code
        }
    }
};
}
```

### Spacing

Spaces are to be added before the opening parenthesis of all control
statements. No spaces should be present preceeding or trailing in argument
lists, template specification, array indexing or between any set of brackets.

Spaces should additionally be present in between all binary, ternary and
assignment operators, but should **not** be present in unary operators between
the operator and the operand.

Inline comments have to be one space away from the end of the statement unless
being aligned in a group.

The following example demonstrates well formatted code under the spacing rules:

```c++
void foo(int a, int b)
{
    int x = 0;
    int y = 0; // Inline comments have to be at least one space away

    ++x;                   // Example unary operator
    int z = x + y;         // Example binary operator
    int x = z > 2 ? 1 : 3; // Example ternary operator

    if (z >= 1) {
        // Some code here
    } else {
        // More code here
    }

    for (std::size_t i = 0; i < 56; ++i) {
        // For loop body
    }

    while (true) {
        // While loop body
    }
}

template <typename T>
void bar(T a)
{
    std::vector<T> foo{a};

    std::cout << foo[0] << std::endl;
}
```

### Alignment

If an argument list is to be split into multiple lines, the subsequent
arguments must be aligned with the opening brace of the argument list.
Alignment should also be performed on multiline binary or ternary operations.

If multiple trailing inline comments are used, they should all be aligned
together.

The following example demonstrates well formatted code under the alignment
rules:

```c++
void foo()
{
    int a = 2;  // Inline comments
    int b = 3;  // must be aligned
    int c = a + // Multiline binary operator has to be aligned.
            b;
}

void bar(int a, int b, int c, int d, int e, int f)
{
    // Function body here
}
```

### Braces and Line Breaks

The line length limit is set to around 100 characters. This means that most
expressions approaching 100 characters should be broken into multiline
statements. Clang-format will attempt to target this limit, going over the
limit slightly if there are tokens that should not be split. Comments should
wrap around unless they include elements that cannot be split (e.g. URLs).

Line breaks should be added before braces on enum, function, record definitions
and after all template specializations except for the `extern "C"` specifier.
Lambdas have special rules that need to be handled separately, see section
below.

Other control structures should not have line breaks before braces.

Braces should be added for all control structures, even those whose bodies only
contain a single line.

**Note: Clang-format does not have the ability to enforce brace presence, one
must manually ensure all braces are present before formatting via
clang-format.**

The following example demonstrates well formatted code under the braces and
line break rules:

```c++
extern "C" {
#include <foo.h>
}

namespace Foo
{
struct Bar
{
    void foobar();
};

template <class T>
void example(T veryLongArgumentName, T anotherVeryLongArgumentName, T aThirdVeryLongArgumentName,
             T aForthVeryLongArgumentName, T aFifthVeryLongArgumentName)
{
    // This is a very long comment that has been broken into two lines due to it exceeding the 100
    // characters per line rules.

    if (...) {
        // Single line control statements are still required to use braces.
    } else {
        // Multiline block
        // Multiline block
    }
}
}
```

### Lambdas

Lambdas are to follow special break rules defined by clang-format. In
particular, if the lambda body contains a single statement and line length
permits, the lambda is to be treated as a single expression, represented in an
inlined format (i.e. no newlines). Or else, a newline is to be inserted
**after** the opening bracket.

The following example demonstrates well formatted code under the lambda rules:

```c++
// Empty lambda, all on same line.
auto a = []() {};

// Lambda with single statement, all on same line.
auto b = []() { return 0; };

// Lambda with multiple statements, line break after the opening bracket.
auto swap = [](int& a, int& b) {
    a = a ^ b;
    b = a ^ b;
    a = a ^ b;
};

// Long lambda with single statement, line break after the opening bracket.
auto compareAndUpdate = [](const int expect, int& actual, int& newVal) -> int {
    actual = (actual == expect) ? newVal : actual;
};
```

### Pointers

Pointers, references and rvalue references should be be aligned left, combining
with the type **when it is possible to do so**. What this means that in a
regular pointer declaration of variable `x` pointing to a type `T` should be
declared as `T* x;` where the \* glyph is placed next to the type `T` without any
spaces separating them. A space should be present between pointer type and the
variable name except in the special cases described below.

Special cases exist when the pointer glyph and the variable needs to put in
parentheses such as when declaring pointers to C-style arrays and pointers to
functions. In these cases, the pointer **should be combined with the variable**
and placed one space away from the pointer type, see examples below.

As a reminder, usage of C-style arrays should be minimized and generally
restricted to interactions with C-based APIs present in external libraries.
Consider using the keyword `auto` to allow automatic type deduction by the
compiler to avoid long and messy type ids.

This rule should apply everywhere: function parameters, declarations,
constructor initializer lists, etc, applying even if the variable name is not
specified.

A few examples of pointer specifications is given below:

```c++
int* bar(int* foo)
{
    // Return type pointer binds to type and does not float in the middle.
}

int a = 0;

int* x;     // Pointer is put next to the type
int& y = a; // Reference is put next to the type.

int (*z)[1]; // Special case: pointer binded to 'z' due to requirement of being in paratheses.

int* (*a)(int*) = &bar; // Pointer binded to 'a' due to require of being in paratheses, rest of the
                        // type maintains pointer being next to the type.

void foo(int* x, int&&); // Forward function declaration pointers and rvalue references bind to type
                         // even if there is no name.
```

To avoid side effects, calling a static member function via an object-expression
is not allowed. The only allowed way is via the class name.

```c++
class foo {
public:
   static void bar();
};

foo& g();

void f()
{
   foo::bar(); // OK: no object necessary
   g().bar(); // Not OK: g() can cause side effects
}
```

### Unary Increment/Decrement

When the use of the prefix and postfix notation for increment and decrement
operators yield the same effect (typical when the return value is ignored), the
prefix notation is preferred to ensure a consistent style. This applies to all
uses of the increment/decrement operators, including those embedded in
for-loops.

A few examples of the usage of increment/decrement operators:

```c++
int a = 0;

++a; // Preferred over "a++".

// Usage of ++i rather than i++.
for (std::size_t i = 0; i < 5; ++i) {
    // for-loop code
}

// Allowed since ++a is not equivalent.
if (a++ == 0) {
    // if statement body
}
```

**Note: Clang-format does not have the ability to enforce consistent
prefix/postfix choice, one must manually ensure the correct style is used.**

### Includes

Minimize the amount of include directives used in header files if they can be
placed in the source file (i.e. don't include something used in the source but
not in the header declaration). This helps improve compile times and keeps the
header lean.

Include directives should include header files in the following order:

| Order |    Header Type   | Description                                                                                                                  |
| :---: | :--------------: | :--------------------------------------------------------------------------------------------------------------------------- |
|   1   | Main             | The main header corresponding to a source (e.g. a source file `foo.cpp` includes `foo.h` as it's main header).               |
|   2   | Local/Module     | Headers in the same folder as the current file. These headers should be included directly, without specifying the full path. |
|   3   | Project          | Headers belonging to the qTox project. These should be specified using full header paths starting within "src/".             |
|   4\* | Qt               | Headers for Qt objects and functions.                                                                                        |
|   5\* | Other            | Headers for any other dependencies (external libraries, tox, C/C++ STL, system headers, etc.                                 |

\* These headers should be included with angle bracket (e.g.
`#include <cstdint>`).

For better header sorting, consider additionally sorting headers in the "other"
category (category 5) in the following order: Tox, external libraries, C/C++
STL and system headers for a smaller include profile (this is not mandatory).

Newlines can be present between includes to indicate logical grouping, however
be wary that clang-format does not sort includes properly this way, electing to
sort each group individually according to the criteria defined above.

Use `#pragma once` rather than include guards in header files. It reduces duplication and avoids a potential cause of bugs.

The following example demonstrates the above include rules:

```c++
#include "core.h"

#include "cdata.h"
#include "coreav.h"
#include "corefile.h"
#include "cstring.h"

#include "net/avatarbroadcaster.h"
#include "nexus.h"
#include "persistence/profile.h"
#include "persistence/profilelocker.h"
#include "persistence/settings.h"

#include <QCoreApplication>
#include <QThread>
#include <QTimer>

#include <tox/tox.h>
#include <tox/toxav.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <ctime>
#include <functional>
#include <limits>
```

### Singletons

Do not introduce new singleton classes. Prefer to move code in the direction
of fewer singleton classes over time.

Singletons complicate destruction, complicate making multiple instances of
something in the future, i.e. having two Tox profiles loaded at once is
difficult to implement in qTox because both Settings and Profile are singleton.
Singleton's also make unit testing and reasoning more difficult by more
tightly coupling classes.

## Documentation

When adding new code to qTox also add doxygen style comments before the
implementation. If an old function is changed, make sure the existing
documentation is updated to reflect the changes or if none exists, add it.

Always attempt to put the documentation at the point of implementation (i.e.
put as much in the source `.cpp` files as possible and minimize clutter in `.h`
files.)

The documentation style mandates the use of `/**` to start a doxygen style
comment, and having ` *` (space asterisk) on all lines following the starting
line. Doxygen keywords like `@brief`, `@param` and `@return` should be used
such that doxygen can intelligently generate the appropriate documentation.

On all updates to master, doxygen comments are automatically generated for the
source code, available at https://qtox.github.io/doxygen.

```C++
/*...license info...*/
#include "blabla.h"

/**
 * @brief I can be briefly described as well!
 *
 * And here goes my longer descrption!
 *
 * @param x Description for the first parameter
 * @param y Description for the second paramater
 * @return An amazing result
 */
static int example(int x, int y)
{
    // Function implementation...
}

/**
 * @class OurClass
 * @brief Exists for some reason...!?
 *
 * Longer description
 */

/**
 * @enum OurClass::OurEnum
 * @brief The brief description line.
 *
 * @var EnumValue1
 * means something
 *
 * @var EnumValue2
 * means something else
 *
 * Optional long description
 */

/**
 * @fn OurClass::somethingHappened(const QString &happened)
 * @param[in] happened    tells what has happened...
 * @brief This signal is emitted when something has happened in the class.
 *
 * Here's an optional longer description of what the signal additionally does.
 */
```

## No translatable HTML tags

Do not put HTML in UI files, or inside Qt's `tr()`. Instead, you can embed HTML
directly into C++ in the following way, to make only the user-facing text
translatable:

```C++
someWidget->setTooltip(QStringLiteral("<html><!-- some HTML text -->") + tr("Translatable text…")
                       + QStringLiteral("</html>"));
```

## Strings

* Use `QStringLiteral` macro when creating new string.

In this example, string is not intended to be modified or copied (like
appending) into other string:
```
    QApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("qTox"));
```

* Use `QLatin1String` when specialized overload exists.

Overload such as `QString::operator==(QLatin1String)` helps to avoid creating
temporary QString and thus avoids malloc:
```
   if (eventType == QLatin1String("uri"))
        handleToxURI(firstParam.toUtf8());
    else if (eventType == QLatin1String("save"))
        handleToxSave(firstParam.toUtf8());
```

* Use `QStringBuilder` and `QLatin1String` when joining strings (and chars)
together.

`QLatin1String` is literal type and knows string length at compile time
(compared to `QString(const char*)` run-time cost with plain C++
string literal). Also, copying 8-bit latin string requires less memory
bandwith compared to 16-bit `QStringLiteral` mentioned earlier, and
copying here is unavoidable (and thus `QStringLiteral` loses it's purpose).

Include `<QStringBuilder>` and use `%` operator for optimized single-pass
concatenation with help of expression template's lazy evaluation:

```
        if (!dir.rename(logFileDir % QLatin1String("qtox.log"),
                        logFileDir % QLatin1String("qtox.log.1")))
            qCritical() << "Unable to move logs";
```
```
    QCommandLineParser parser;
    parser.setApplicationDescription(QLatin1String("qTox, version: ")
    % QLatin1String(GIT_VERSION) % QLatin1String("\nBuilt: ")
    % QLatin1String(__TIME__) % QLatin1Char(' ') % QLatin1String(__DATE__));
```

* Use `QLatin1Char` to avoid UTF-16-char handling (same as in previous
example):
```
    QString path = QString(__FILE__);
    path = path.left(path.lastIndexOf(QLatin1Char('/')) + 1);
```

* Use `QLatin1String` and `QLatin1Char` _only_ for Latin-1 strings and chars.

[Latin-1][Latin-1] is ASCII-based standard character encoding, use
`QStringLiteral` for Unicode instead.

For more info, see:

* [Using QString Effectively]
* [QStringLiteral explained]
* [String concatenation with QStringBuilder]

<!-- Markdown links -->
[ISO/IEC/C++11]: http://www.iso.org/iso/catalogue_detail.htm?csnumber=50372
[Exceptions]: https://en.wikipedia.org/wiki/C%2B%2B#Exception_handling
[RTTI]: https://en.wikipedia.org/wiki/Run-time_type_information
[`tools/format-code.sh`]: /tools/format-code.sh
[Using QString Effectively]: https://wiki.qt.io/Using_QString_Effectively
[QStringLiteral explained]: https://woboq.com/blog/qstringliteral.html
[String concatenation with QStringBuilder]: https://blog.qt.io/blog/2011/06/13/string-concatenation-with-qstringbuilder/
[Latin-1]: https://en.wikipedia.org/wiki/ISO/IEC_8859-1
[CppCoreGuidelines]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#main
