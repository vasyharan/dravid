#ifndef LANG_COMPILER_STACK_H
#define LANG_COMPILER_STACK_H

#include <stack>

namespace lang {
namespace compiler {

template <typename _Tp> class stack {
  std::stack<_Tp> _stack;

public:
  _Tp &pop() {
    auto top = this->top();
    _stack.pop();
    return top;
  }

  _Tp &push(_Tp &v) {
    _stack.push(v);
    return this->top();
  }

  _Tp &top() { return _stack.top(); }
};

} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_STACK_H
