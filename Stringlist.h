// Stringlist.h

/////////////////////////////////////////////////////////////////////////
//
//
// Name : Jaime Saraza
//
//
// Statement of Originality
// ------------------------
//
// All the code and comments below are my own original work. For any non-
// original work, I have provided citations in the comments with enough
// detail so that someone can see the exact source and extent of the
// borrowed work.
//
//
/////////////////////////////////////////////////////////////////////////

#pragma once

#include <cassert>
#include <iostream>
#include <string>

using namespace std;

class Stringlist
{
    struct Node
    {
        int node_index = 0;
        string undo_command;
        string value;
        int string_index = 0;
        Node *next;
        Node *prev;
        int capc;
        int sze;
    };

    Node *head = nullptr;
    Node *tail = nullptr;

    int cap;     // capacity
    string *arr; // array of strings
    int sz;      // size
    //
    // Helper function for throwing out_of_range exceptions.
    //
    void bounds_error(const string &s) const
    {
        throw out_of_range("Stringlist::" + s + " index out of bounds");
    }

    //
    // Helper function for checking index bounds.
    //
    void check_bounds(const string &s, int i) const
    {
        if (i < 0 || i >= sz)
        {
            bounds_error(s);
        }
    }

    //
    // Helper function for copying another array of strings.
    //
    void copy(const string *other)
    {
        for (int i = 0; i < sz; i++)
        {
            arr[i] = other[i];
        }
    }

    //
    // Helper function for checking capacity; doubles size of the underlying
    // array if necessary.
    //
    void check_capacity()
    {
        if (sz == cap)
        {
            cap *= 2;
            string *temp = new string[cap];
            for (int i = 0; i < sz; i++)
            {
                temp[i] = arr[i];
            }
            delete[] arr;
            arr = temp;
            delete[] temp;
            temp = nullptr;
        }
    }

public:
    //
    // Default constructor: makes an empty StringList.
    //
    Stringlist()
        : head(nullptr), tail(nullptr), cap(10), arr(new string[cap]), sz(0)
    {
    }

    //
    // Copy constructor: makes a copy of the given StringList.
    //
    // Does *not* copy the undo stack, or any undo information from other.
    //
    Stringlist(const Stringlist &other)
        : cap(other.cap), arr(new string[cap]), sz(other.sz)
    {
        copy(other.arr);
    }

    //
    // destructor
    //
    ~Stringlist()
    {
        // delete[] arr;       //deletes existing array
        if (head != nullptr)
        {
            while (head != nullptr) // loops over everysingle node in the call stack and deletes it
            {
                remove_node_from_stack(head);
            }
        }
        head = nullptr;
        tail = nullptr;
    }

    //
    // Assignment operator: makes a copy of the given StringList.
    //
    // undoable
    //
    // For undoing, when assigning different lists, the undo stack is NOT
    // copied:
    //
    //    lst1 = lst2; // lst1 undo stack is updated to be able to undo the
    //                 // assignment; lst1 does not copy lst2's stack
    //                 //
    //                 // lst2 is not change in any way
    //
    // Self-assignment is when you assign a list to itself:
    //
    //    lst1 = lst1;
    //
    // In this case, nothing happens to lst1. Both its string data and undo
    // stack are left as-is.
    //
    Stringlist &operator=(const Stringlist &other)
    {
        // adds every single node that is being deleted to the call stack when this object is assigned to another
        for (int i = sz - 1; i >= 0; i--)
        {
            add_stack(head, tail, arr[i], i, "EQUALS", cap, sz);
        }
        // deletes existing array in the object and assigns new values from the other object
        if (this != &other)
        {
            delete[] arr;
            cap = other.capacity();
            arr = new string[cap];
            sz = other.size();
            copy(other.arr);
        }
        return *this;
    }

    //
    // Returns the number of strings in the list.
    //
    int size() const
    {
        return sz;
    }

    //
    // Returns true if the list is empty, false otherwise.
    //
    bool empty() const { return size() == 0; }

    //
    // Returns the capacity of the list, i.e. the size of the underlying array.
    //
    int capacity() const { return cap; }

    //
    // Returns the string at the given index.
    //
    string get(int index) const
    {
        check_bounds("get", index);
        return arr[index];
    }

    //
    // Returns the index of the first occurrence of s in the list, or -1 if s is
    // not in the lst.
    //
    int index_of(const string &s) const
    {
        for (int i = 0; i < sz; i++)
        {
            if (arr[i] == s)
            {
                return i;
            }
        }
        return -1;
    }

    //
    // Returns true if s is in the list, false otherwise.
    //
    bool contains(const string &s) const
    {
        return index_of(s) != -1;
    }

    //
    // Returns a string representation of the list.
    //
    string to_string() const
    {
        string result = "{";
        for (int i = 0; i < size(); i++)
        {
            if (i > 0)
            {
                result += ", ";
            }
            result += "\"" + get(i) + "\"";
        }
        return result + "}";
    }

    //
    // Sets the string at the given index.
    //
    // undoable
    //
    void set(int index, string value)
    {
        add_stack(head, tail, arr[index], index, "SET", cap, sz);
        check_bounds("set", index);
        arr[index] = value;
    }

    //
    // Insert s before index; if necessary, the capacity of the underlying array
    // is doubled.
    //
    // undoable
    //
    void insert_before(int index, const string &s)
    {
        //
        // adds to the call stack what was inserted
        //
        add_stack(head, tail, s, index, "INSERT", cap, sz);
        //
        // checks if index is within the boundary of our array
        //
        if (index < 0 || index > sz)
        { // allows insert at end, i == sz
            bounds_error("insert_before");
        }
        check_capacity();

        //
        // moves every single element in the arry from the index over to the right
        for (int i = sz; i > index; i--)
        {
            arr[i] = arr[i - 1];
        }
        //
        // assigns insert to corresponding index
        //
        arr[index] = s;

        //
        // increases size by 1
        //
        sz++;
    }

    //
    // Appends s to the end of the list; if necessary, the capacity of the
    // underlying array is doubled.
    //
    // undoable
    //
    void insert_back(const string &s)
    {
        insert_before(size(), s);
    }

    //
    // Inserts s at the front of the list; if necessary, the capacity of the
    // underlying array is doubled.
    //
    // undoable
    //
    void insert_front(const string &s)
    {
        insert_before(0, s);
    }

    //
    // Removes the string at the given index; doesn't change the capacity.
    //
    // undoable
    //
    void remove_at(int index)
    {
        //
        /// adds to the call stack the sign to remove, the index it was removed at and the value it was removed at
        //
        add_stack(head, tail, arr[index], index, "REMOVE", cap, sz);
        //
        /// checks if its within bounds before deleting
        //
        check_bounds("remove_at", index);
        //
        /// if index has one element there is no reason to move other elements because there are no other elements
        /// so we deleteone element
        //
        if (index == 0 && sz == 1)
        {
            arr[index] == "";
        }
        else
        {
            //
            /// moves everything to the left
            //
            for (int i = index; i < sz - 1; i++)
            {
                arr[i] = arr[i + 1];
            }
        }
        //
        /// decreases size by one
        //
        sz--;
    }

    //
    // Removes all strings from the list; doesn't change the capacity.
    //
    // undoable
    //
    void remove_all()
    {
        //
        /// creates a loop to remove everything one by one until the size of the array is 0
        /// calls each element onto a call stack
        /// decreases sz one by one until sz == 0
        //
        while (sz > 0)
        {
            add_stack(head, tail, arr[sz - 1], sz - 1, "REMOVEALL", cap, sz);
            arr[sz] = "";
            sz--;
        }
    }

    //
    // Removes the first occurrence of s in the list, and returns true. If s is
    // nowhere in the list, nothing is removed and false is returned.
    //
    // undoable
    //
    bool remove_first(const string &s)
    {
        int index = index_of(s);
        if (index == -1)
        {
            return false;
        }
        remove_at(index);
        return true;
    }

    //
    // Undoes the last operation that modified the list. Returns true if a
    // change was undone.
    //
    // If there is nothing to undo, does nothing and returns false.
    //
    bool undo()
    {
        if (undo_set() == true)
        {
            return true;
        }
        else if (undo_insert() == true)
        {
            return true;
        }
        else if (undo_removeAt() == true)
        {
            return true;
        }
        else if (undo_remove_all() == true)
        {
            return true;
        }
        else if (undo_equals() == true)
        {
            return true;
        }
        cout << "Stringlist::undo: not yet implemented\n";
        return false;
    }

    bool undo_set()
    {
        if (tail == nullptr)
        {
            return false;
        }
        else
        {
            //
            /// calls node if the undo command is SET
            /// keeps the value that was deleted when the element was set to something else
            /// puts the value back and removes the node from the call stack
            //
            if (tail->undo_command == "SET")
            {
                arr[tail->string_index] = tail->value;
                remove_node_from_stack(head);
                return true;
            }
        }
        return false;
    }

    bool undo_insert()
    {
        if (tail == nullptr)
        {
            return false;
        }
        else
        {
            //
            /// calls node if the command is INSERT
            /// deletes the node at corresponding index and decreases sz
            //
            /// if the element was the only one deleted we can remove the node from the call stack and exit the method
            /// however if there are other elements left in the array we will move each element to the right

            /// moving each element to the right will fill in the gap created by deleting the element
            //
            if (tail->undo_command == "INSERT")
            {
                arr[tail->string_index] = "";
                sz--;
                if (sz == 0)
                {
                    remove_node_from_stack(head);
                    return true;
                }
                else
                {
                    for (int i = tail->string_index; i < sz; i++)
                    {
                        arr[i] = arr[i + 1];
                    }
                }
                remove_node_from_stack(head);
                return true;
            }
        }
        return false;
    }

    //
    /////                     //for the method below//
    //
    /// if the command in the call stack is REMOVEALL we can continue into a while loop
    //
    /// we will keep adding the elements back in the array until the call command changes
    //

    bool undo_remove_all()
    {
        if (tail == nullptr)
        {
            return false;
        }
        else if (tail->undo_command == "REMOVEALL")
        {
            while (tail->undo_command == "REMOVEALL")
            {
                if (sz < 1)
                {
                    arr[0] = tail->value;
                }
                else
                {
                    if (sz == 1)
                    {
                        arr[tail->string_index] = tail->value;
                    }
                    else
                    {
                        for (int i = sz - 1; i > tail->string_index; i--)
                        {
                            arr[i] = arr[i - 1];
                        }
                        arr[tail->string_index] = tail->value;
                    }
                }
                remove_node_from_stack(head);
                this->sz++;
            }
            return true;
        }
        return false;
    }

    //
    /// if the command in the call stack is REMOVE we will add the element we removed back into the corresponding indice
    //
    /// we will then move every element over to the right
    /// remove the node frrom the stack
    /// then decrement sz by one
    //

    bool undo_removeAt()
    {
        if (tail == nullptr)
        {
            return false;
        }
        else
        {
            if (tail->undo_command == "REMOVE")
            {
                if (sz < 1)
                {
                    arr[0] = tail->value;
                }
                else
                {
                    if (sz == 1)
                    {
                        arr[tail->string_index] = tail->value;
                    }
                    else
                    {
                        for (int i = sz - 1; i > tail->string_index; i--)
                        {
                            arr[i] = arr[i - 1];
                        }
                        arr[tail->string_index] = tail->value;
                    }
                }

                remove_node_from_stack(head);
                this->sz++;
                return true;
            }
        }
        return false;
    }

    //
    /// to undo the equals operator we will delete the current array and recreate the cap and sz
    //
    /// the call stack of the object being changed is untouched
    /// therefore we can recreate the object from before when it was deleted and assigned as another object
    //
    /// while the call stack command is EQUALS we can set each element in the array back to what it previously was
    /// one by one until it is back to normal and each time we will increase sz
    //

    bool undo_equals()
    {
        delete[] arr;
        arr = new string[cap];
        cap = 10;
        sz = 0;

        if (tail == nullptr)
        {
            return true;
        }
        else
        {
            while (tail != nullptr && tail->undo_command == "EQUALS")
            {
                arr[tail->string_index] = tail->value;
                remove_node_from_stack(head);
                sz++;
            }
            return true;
        }
        return false;
    }

    // this function below deletes the last node from the stack
    // Citations for help: https://www.javatpoint.com/deletion-in-doubly-linked-list-at-the-end
    // https://www.alphacodingskills.com/cpp/ds/cpp-delete-the-last-node-of-the-doubly-linked-list.php

    void remove_node_from_stack(Node *head)
    {
        if (head == nullptr || tail == nullptr)
        {
            this->head = nullptr;
            this->tail = nullptr;
            return;
        }
        else if (tail == head)
        {
            delete tail;
            tail = nullptr;
            head = nullptr;
        }
        else
        {
            Node *deleteNode = tail;
            tail = tail->prev;
            tail->next = nullptr;
            delete deleteNode;
            deleteNode = nullptr;
        }
        return;
    }

    // this function is to get the current number of elements in the stack to help navigate the index of the stack
    int get_stack_size(Node *head)
    {
        Node *current = head;
        int current_index = 0;

        while (current != nullptr)
        {
            current_index++;
            current = current->next;
        }
        return current_index;
    }

    void add_stack(Node *&head, Node *&tail, string value, int index, string undo_command, int capc, int sze)
    {
        Node *newNode = new Node{get_stack_size(head) + 1, undo_command, value, index, nullptr, nullptr, capc, sze};
        if (head == nullptr && tail == nullptr)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            newNode->prev = tail;
            tail->next = newNode;
            tail = newNode;
        }
    }

}; // class Stringlist

//
// Prints list to in the format {"a", "b", "c"}.
//
ostream &operator<<(ostream &os, const Stringlist &lst)
{
    return os << lst.to_string();
}

//
// Returns true if the two lists are equal, false otherwise.
//
// Does *not* consider any undo information when comparing two Stringlists. All
// that matters is that they have the same strings in the same order.
//
bool operator==(const Stringlist &a, const Stringlist &b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    for (int i = 0; i < a.size(); i++)
    {
        if (a.get(i) != b.get(i))
        {
            return false;
        }
    }
    return true;
}

//
// Returns true if the two lists are not equal, false otherwise.
//
// Does *not* consider any undo information when comparing two Stringlists.
//
bool operator!=(const Stringlist &a, const Stringlist &b)
{
    return !(a == b);
}