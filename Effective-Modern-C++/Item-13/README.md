### Item 13: Prefer **const_iterator**s to **iterator**s

#### Overall
- Prefer **const_iterator**s to **iterator**s.
- In maximally generic code, prefer non-member versions of **begin**, **end**, **rbegin**, etc., over their member function counterparts.

This item is supposed to be short. Just like programmers should use **const** whenever, they should prefer **const_iterator**s over **iterator**s whenever possible.

However, it is such a pain to work with **const_iterator** in C++98 that many people abandon its usage in the end. Suppose we want to find a target in the **std::vector** and insert a value in front of it. In order to utilize **const_itetator**, people have to write:

```CPP
typedef std::vector<int>::iterator IterT;
typedef std::vector<int>::const_iterator ConstIterT;

std::vector<int> values;
...
ConstIterT ci = std::find(static_cast<ConstIterT>(values.begin()),
			  static_cast<ConstIterT>(values.end()),
			  1983);
values.insert(static_cast<IterT>(ci), 1998);
```

The pains lie in several points: It's hard to get **const_iterator** from non-**const** container. And insertion could only accept **iterator**, not **const_iterator**.

All the above pains go away with the introduction of C++11/14. We could revise the code as follow:

```CPP
std::vector<int> values;
...
auto it = std::find(values.cbegin(), values.cend(), 1983);
values.insert(it, 1998);
```

Amazingly convenient and conform to the "using **const** whenever possible" idiom, right?

Besides, when we are working on generic code across different versions of C++, it's best to prefer *non-member* iterator function over *member* function. That's the case for built-in arrays and thrid-party libraries. For example, we could generalize our code into a template as follows:

```CPP
// if C++11, cbegin has not become non-member, we can define our own version
template<class C>
auto cbegin(const C& container) -> decltype(std::begin(container)) {
  return std::begin(container);
}

// in C++14, fully available
template<typename C, typename V>
void findAndInsert(C& container,
		   const V& targetVal,
		   const V& insertVal) {
  using std::cbegin;
  using std::cend;

  auto it = std::find(cbegin(container), cend(container), targetVal);
  container.insert(it, insertVal);
}
```