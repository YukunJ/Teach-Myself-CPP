### How to use Google's Protocol Buffer in C++

#### Why **Protocol Buffer**?

Imagine we need to serialize/deserialize a data structure, there are a few feasible ways:

+ save the raw in-memory data structure into binary form. But this is a fragile approach as it requires the reading side must be complied with exactly the same memory layout and prohibits the extension of the data format.
+ write our own encoding schema, for example as a colon-separated string "12:3:-23:67", which requires us to write the encoding and parsing code, which also brings runtime overhead.
+ serialize the data to XML form. This is widely used format that's human readable. However, XML takes huge amount of storage space for being verbose.

Here comes Google's **Protocol Buffer**: flexible, efficient, automated solution.

#### Define Message

Here we will see how to define a Protocol Buffer message format. See the [addressbook.proto](./addressbook.proto) for a complete reference.

```CPP
syntax = "proto2";

package tutorial;

message Person {
    optional string name = 1;
    optional int32 id = 2;
    optional string email = 3;

    enum PhoneType {
        MOBILE = 0;
        HOME = 1;
        WORK = 2;
    }

    message PhoneNumber {
        optional string number = 1;
        optional PhoneType type = 2 [default = HOME];
    }

    repeated PhoneNumber phones = 4;
}

message AddressBook {
    repeated Person people = 1;
}
```

We see the message format definition is very close to a `class`/`struct` defintion in C/C++. Quite a few primitive data types are available like `int32`, `string`, and we can nest customized data type, like `PhoneType` nested inside `PhoneNumber`, and `Person` nested inside `AddressBook`.

Each field must have one of the three allowed modifiers: `required`, `optional`, `repeated`. `required` should be rarely used because it might hurt the future portability of the message format. `repeated` is when a data field might appear continuously for a few times, just like an dynamic array. `optional`, as its name indicates, suggests this field might or might not be set. Whenr retrieving value from an unset field, the system default value is return, 0 for Integer and empty for string, etc.

#### Compile Message
We first install the protocol buffer software on our machine. On my Linux Ubuntu 20.04 LTS, it could easily be done via 

```bash
$ sudo apt install -y protobuf-compiler
```

and we check its version is up to date

```bash
$ protoc --version
libprotoc 3.6.1
```

Now we can compile the protobuf ßmessage format into cpp file. Let the compiler does the code generation for us.

```bash
$ ls
README.md addressbook.proto
$ protoc --cpp_out=. addressbook.proto // compile into current directory
$ ls
README.md addressbook.pb.cc addressbook.pb.h addressbook.proto
```

We see the compiler generates `addressbook.ph.h` and `addressbook.ph.cc` for us. If we take a look inside, it contains a lot of getter and setter functions for the message format we just defined above.

#### Common Message Methods

There are a few commonly used methods universe across all versions of protobuf.

+ `bool IsInitialized() const`: checks if all the required fields have been set.
+ `string DebugString() const`:  returns a human-readable representation of the message, particularly useful for debugging.
+ `void CopyFrom(const Person& from)`: overwrites the message with the given message’s values.
+ `void Clear()`: clears all the elements back to the empty state.
+ `bool SerializeToString(string* output) const`: serializes the message and stores the bytes in the given string. Note that the bytes are binary, not text; we only use the `string` class as a convenient container.
+ `bool ParseFromString(const string& data)`:  parses a message from the given string.
+ `bool SerializeToOstream(ostream* output) const`:  writes the message to the given C++ `ostream`.
+ `bool ParseFromIstream(istream* input)`: parses a message from the given C++ `istream`.

#### Saving a Message

Here we write a simple program to append a new person's address information into the data file.

```CPP
#include <iostream>
#include <fstream>
#include <string>
#include "addressbook.pb.h"
using namespace std;

// This function fills in a Person message based on user input.
void PromptForAddress(tutorial::Person* person) {
  cout << "Enter person ID number: ";
  int id;
  cin >> id;
  person->set_id(id);
  cin.ignore(256, '\n');

  cout << "Enter name: ";
  getline(cin, *person->mutable_name());

  cout << "Enter email address (blank for none): ";
  string email;
  getline(cin, email);
  if (!email.empty()) {
    person->set_email(email);
  }

  while (true) {
    cout << "Enter a phone number (or leave blank to finish): ";
    string number;
    getline(cin, number);
    if (number.empty()) {
      break;
    }

    tutorial::Person::PhoneNumber* phone_number = person->add_phones();
    phone_number->set_number(number);

    cout << "Is this a mobile, home, or work phone? ";
    string type;
    getline(cin, type);
    if (type == "mobile") {
      phone_number->set_type(tutorial::Person::MOBILE);
    } else if (type == "home") {
      phone_number->set_type(tutorial::Person::HOME);
    } else if (type == "work") {
      phone_number->set_type(tutorial::Person::WORK);
    } else {
      cout << "Unknown phone type.  Using default." << endl;
    }
  }
}

// Main function:  Reads the entire address book from a file,
//   adds one person based on user input, then writes it back out to the same
//   file.
int main(int argc, char* argv[]) {
  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  if (argc != 2) {
    cerr << "Usage:  " << argv[0] << " ADDRESS_BOOK_FILE" << endl;
    return -1;
  }

  tutorial::AddressBook address_book;

  {
    // Read the existing address book.
    fstream input(argv[1], ios::in | ios::binary);
    if (!input) {
      cout << argv[1] << ": File not found.  Creating a new file." << endl;
    } else if (!address_book.ParseFromIstream(&input)) {
      cerr << "Failed to parse address book." << endl;
      return -1;
    }
  }

  // Add an address.
  PromptForAddress(address_book.add_people());

  {
    // Write the new address book back to disk.
    fstream output(argv[1], ios::out | ios::trunc | ios::binary);
    if (!address_book.SerializeToOstream(&output)) {
      cerr << "Failed to write address book." << endl;
      return -1;
    }
  }

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
```

#### Reading a Message

Here we read back the data file written above and exhibit all information contained inside.

```CPP
#include <iostream>
#include <fstream>
#include <string>
#include "addressbook.pb.h"
using namespace std;

// Iterates though all people in the AddressBook and prints info about them.
void ListPeople(const tutorial::AddressBook& address_book) {
  for (int i = 0; i < address_book.people_size(); i++) {
    const tutorial::Person& person = address_book.people(i);

    cout << "Person ID: " << person.id() << endl;
    cout << "  Name: " << person.name() << endl;
    if (person.has_email()) {
      cout << "  E-mail address: " << person.email() << endl;
    }

    for (int j = 0; j < person.phones_size(); j++) {
      const tutorial::Person::PhoneNumber& phone_number = person.phones(j);

      switch (phone_number.type()) {
        case tutorial::Person::MOBILE:
          cout << "  Mobile phone #: ";
          break;
        case tutorial::Person::HOME:
          cout << "  Home phone #: ";
          break;
        case tutorial::Person::WORK:
          cout << "  Work phone #: ";
          break;
      }
      cout << phone_number.number() << endl;
    }
  }
}

// Main function:  Reads the entire address book from a file and prints all
//   the information inside.
int main(int argc, char* argv[]) {
  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  if (argc != 2) {
    cerr << "Usage:  " << argv[0] << " ADDRESS_BOOK_FILE" << endl;
    return -1;
  }

  tutorial::AddressBook address_book;

  {
    // Read the existing address book.
    fstream input(argv[1], ios::in | ios::binary);
    if (!address_book.ParseFromIstream(&input)) {
      cerr << "Failed to parse address book." << endl;
      return -1;
    }
  }

  ListPeople(address_book);

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
```

#### Compile & Run Program

Finally, we are ready to compile our reader and writer program using the Google's Protocol Buffer library.

```bash
$ c++ -std=c++14 writer.cpp addressbook.pb.cc -o writer -lpthread -lprotobuf
$ c++ -std=c++14 reader.cpp addressbook.pb.cc -o reader -lpthread -lprotobuf
```

Let's try it out:

```bash
$ ./writer addressbook.data
addressbook.data: File not found.  Creating a new file.
Enter person ID number: 1234
Enter name: Tommy
Enter email address (blank for none): tommy@gmail.com
Enter a phone number (or leave blank to finish): 123-456-7890
Is this a mobile, home, or work phone? WORK
Unknown phone type.  Using default.
Enter a phone number (or leave blank to finish): ^C
ubuntu@ip-172-31-30-171:~/protobuf/examples$ ./add_person_cpp new.data
new.data: File not found.  Creating a new file.
Enter person ID number: 1234
Enter name: Tommy
Enter email address (blank for none): tommy@gmail.com
Enter a phone number (or leave blank to finish): 123-456-7890
Is this a mobile, home, or work phone? mobile
Enter a phone number (or leave blank to finish): 

$ ./reader addressbook.data
Person ID: 1234
  Name: Tommy
  E-mail address: tommy@gmail.com
  Mobile phone #: 123-456-7890
  Updated: 2023-07-03T16:18:55Z
```
