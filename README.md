## An Undoable List

> In software, 'undo' erases our mistakes; in life, mistakes craft our story.
> Imagine the tales we'd lose with a real-life Ctrl+Z. -- _ChatGPT_

This projects will add an _undo_ feature to a list of strings.

To start, here's a working class called [Stringlist](Stringlist.h) that
implements a string list as a dynamic array.
[Stringlist_test.cpp](Stringlist_test.cpp) has tests for all the methods in
[Stringlist](Stringlist.h).

This project will implement the function `undo`, thus making `Stringlist` an _undoable_ list.

When it's done, you'll be able to test it like this:

```cpp
#include "Stringlist.h"
#include <iostream>

using namespace std;

int main() {
    Stringlist lst;
    cout << lst << endl; // {}

    lst.insert_back("one");
    lst.insert_back("two");
    lst.insert_back("three");
    cout << lst << endl; // {"one", "two", "three"}

    lst.undo();
    cout << lst << endl; // {"one", "two"}

    lst.undo();
    cout << lst << endl; // {"one"}

    lst.undo();
    cout << lst << endl; // {}
}
```

## Getting Started with `StringList`

To start,
[download all the files for this assignment], and then compile and run
[Stringlist_test.cpp](Stringlist_test.cpp) to make sure it runs without error:

```bash
> make Stringlist_test
g++ -std=c++17 -Wall -Wextra -Werror -Wfatal-errors -Wno-sign-compare -Wnon-virtual-dtor -g Stringlist_test.cpp -o Stringlist_test

> valgrind ./Stringlist_test
... test output ...
```

> **Note** There should be _no_ errors when you run this testing! If you have
> any, double-check that you are running exactly the correct file, and using the
> correct compiler version, correct options, and correct [makefile](makefile).

## Designing the Undo Stack

The implemented `undo()` method is using at least one _private
stack_ implemented as a linked list inside the `Stringlist` class.

The main idea for how undo works is that every time `Stringlist` is modified by
one a method that can be undone, it _pushes_ the _inverse_ operation on the top
of an undo stack. When `undo()` is called, it _pops_ the top of the stack and
applies that operation to the list, thus undoing the most recent operation.

**Important** All the methods in [Stringlist](Stringlist.h) marked "undoable"
should work with `undo()`. Note that `undo()` cannot be undone: there is no
"re-do" feature in this assignment.

Here are some examples of how specific methods should work.

### Undoing `insert_before`

Suppose `lst` is `{"dog", "cat", "tree"}`, and you call
`lst.insert_before(3, "hat")`, making it `{"dog", "cat", "tree", "hat"}`.
It should push the operation _remove string at index 3_ on the undo stack.
You could store it as a short `string` command, e.g. `REMOVE 3`. If you now
call `lst.undo()`, the `REMOVE 3` command on top of the stack is popped and
applied to the list, e.g. the string at index 3 is removed, leaving the list
in the state it was in before calling `insert_before`: `{"dog", "cat", "tree"}`.

In code:

```cpp
// lst == {"dog", "cat", "tree"}

lst.insert_before(3, "hat");
// lst == {"dog", "cat", "tree", "hat"}

lst.undo();
// lst == {"dog", "cat", "tree"}

lst.insert_before(1, "shoe");
// lst == {"dog", "shoe", "cat", "tree"}

lst.undo();
// lst == {"dog", "cat", "tree"}
```

### Undoing `set`

For `set`, suppose that `lst` is `{"yellow", "green", " red cat ", "orange"}`, and so
`lst.get(2)` returns `" red cat "`. If you call `lst.set(2, "cow")`, then you should
push the operation _set location 2 to `"red"`_ onto the undo stack, and then
over-write location 2 with `"cow"`. You could format the operation like
`"SET 2  red cat "`. Calling `lst.undo()` pops the top command of the stack and
applies it to the list, e.g. the string at index 2 is set to `" red cat "` and the
list is in the state it was in before calling `set`:
`{"yellow", "green", " red cat ", "orange"}`.

In code:

```cpp
// lst == {"yellow", "green", " red cat ", "orange"}

lst.set(2, "cow");
// lst == {"yellow", "green", "cow", "orange"}

lst.undo();
// lst == {"yellow", "green", " red cat ", "orange"}
```

### Undoing `remove_at`

For `remove_at`, suppose `lst` is `{"dog", " pink  cat ", "tree"}`. If you call
`lst.remove_at(1)`, you should then push the operation _insert `" pink  cat "` at index 1_
onto the stack, and then remove the string at index 1 so it becomes
`{"dog", " pink  cat ", "tree"}`. You could format the operation as `"INSERT 1  pink  cat "`.
If you now call `lst.undo()`, the command on top of the stack is popped and
applied to the list, e.g. the string `" pink  cat "` is inserted at index 1, and the
list is in the state it was in before calling `remove_at`: `{"dog", " pink  cat ", "tree"}`.

In code:

```cpp
// lst == {"dog", " pink  cat ", "tree"}

lst.remove_at(1);
// lst == {"dog", "tree"}

lst.undo();
// lst == {"dog", " pink  cat ", "tree"}
```

### Undoing `operator=`

For `operator=`, suppose `lst1` is `{"dog", "cat", "tree"}`, and `lst2` is
`{"yellow", "green", "red", "orange"}`. If you call `lst1 = lst2;`, then you
should push the command _set `lst1` to `{"dog", "cat", "tree"}`_ onto the stack,
and then assign `lst1` to `lst2`. Calling `lst1.undo()` pops the command on top
of the stack and applies it to the list, e.g. `lst1` is set to the state it was
in before calling `operator=`: `{"dog", "cat", "tree"}`.

In code:

```cpp
// lst1 == {"dog", "cat", "tree"}
// lst2 == {"yellow", "green", "red", "orange"}

lst1 = lst2;
// lst1 == {"yellow", "green", "red", "orange"}
// lst2 == {"yellow", "green", "red", "orange"}

lst1.undo();
// lst1 == {"dog", "cat", "tree"}
// lst2 == {"yellow", "green", "red", "orange"}
```

As this shows, when you undo `operator=`, the _entire_ list of strings is
restored in _one_ call to `undo()`.

**Important** notes:

- If `lst1` and `lst2` are different objects, then when `lst2` is assigned to
  `lst1` just the underlying string array of `lst2` is copied to `lst1`. The
  `lst1` undo stack is updated so that it can undo the assignment. The undo
  stack of `lst2` is _not_ copied, and `lst2` is not modified in any away.

- _Self-assignment_ is when you assign a list to itself, e.g. `lst1 = lst1;`. In
  this case, _nothing_ happens to `lst1`. Both its string data and undo stack
  are left as-is.

### Undoing `remove_all`

For `remove_all`, suppose `lst` is `{"dog", "cat", "tree"}`. If you call
`lst.remove_all()`, then you should push the operation _set `lst1` to
`{"dog", "cat", "tree"}`_ onto the stack, and then remove all the strings from `lst`
(i.e. its size is 0). Calling `lst1.undo()` pops the command on top of the stack
and applies it to the list, e.g. `lst` is set to `{"dog", "cat", "tree"}` and
the list is in the state it was in before calling `remove_all`:
`{"dog", "cat", "tree"}`.

In code:

```cpp
// lst == {"dog", "cat", "tree"}

lst.remove_all();
// lst == {}

lst.undo();
// lst == {"dog", "cat", "tree"}
```

Notice how _all_ the strings are restored in a single call to `undo()`.

It `lst` is empty, it works like this:

```cpp
// lst == {}

lst.remove_all();
// lst == {}

lst.undo();
// lst == {}
```

### Undoing Other Methods

`undo()` should undo all the other methods in [Stringlist](Stringlist.h) that
are marked "undoable" in the source code comments.

As mentioned above, `undo()` is _not_ undoable. There is no "re-do" feature in
this assignment.

### Testing Your Code

In [a2_test.cpp](a2_test.cpp), write
code to test your `Stringlist` undo method. Compile and run it like this:

```bash
❯ make a2_test
g++  -std=c++17 -Wall -Wextra -Werror -Wfatal-errors -Wno-sign-compare -Wnon-virtual-dtor -g   a2_test.cpp   -o a2_test

> ./a2_test
... testing output ...
```

### Additional Information

- The objects you store on the undo stack should contain enough information to
  undo the operation that was just performed. You could, for example, your stack
  could store `Action` objects, where `Action` is a private `struct` that stores
  the name of the action and any other information needed for undoing.
