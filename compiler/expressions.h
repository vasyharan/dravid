#ifndef LANG_COMPILER_EXPRESSIONS_H
#define LANG_COMPILER_EXPRESSIONS_H

#include <memory>
#include <ostream>
#include <vector>

/* #define MAKE_VISITABLE \ */
/*   virtual void accept(Visitor &v) const override { v.visit(*this); } */

namespace lang {
namespace compiler {
namespace ast {

class Expression {
public:
  Expression() = default;
  Expression(const Expression &) = delete;
  Expression(Expression &&) = delete;

  virtual ~Expression() = default;

  // virtual void accept(Visitor &visitor) const = 0;
  virtual void print(std::ostream &out, int indent = 0) const = 0;

  friend std::ostream &operator<<(std::ostream &out, const Expression &node) {
    node.print(out);
    return out;
  }
};

class Identifier : public Expression {
  const std::string name_;

public:
  Identifier(const std::string &name) : name_(name) {}
  Identifier(const Identifier &) = delete;
  Identifier(Identifier &&) = delete;

  void print(std::ostream &out, int indent = 0) const override;
};

class Integer : public Expression {
  const long value_;

public:
  Integer(long value) : value_(value) {}
  Integer(const Integer &) = delete;
  Integer(Integer &&) = delete;

  void print(std::ostream &out, int indent = 0) const override;
};

class BinaryExpression : public Expression {
  const char op_;
  std::unique_ptr<const Expression> left_, right_;

public:
  BinaryExpression(char op, std::unique_ptr<const Expression> left,
                   std::unique_ptr<const Expression> right)
      : op_(op), left_(std::move(left)), right_(std::move(right)) {}
  BinaryExpression(const BinaryExpression &) = delete;
  BinaryExpression(BinaryExpression &&) = delete;

  void print(std::ostream &out, int indent = 0) const override;
};

class Value : public Expression {
protected:
  const bool constant_;
  const std::string name_;

  std::unique_ptr<const Expression> value_;

public:
  Value(bool constant, const std::string &name)
      : constant_(constant), name_(name), value_(nullptr) {}
  Value(bool constant, const std::string &name,
        std::unique_ptr<const Expression> value)
      : constant_(constant), name_(name), value_(std::move(value)) {}
  Value(const Value &) = delete;
  Value(Value &&) = delete;

  virtual void print(std::ostream &out, int indent = 0) const override;
};

class Parameter : public Value {
public:
  Parameter(bool constant, const std::string &name) : Value(constant, name) {}
  Parameter(const Parameter &) = delete;
  Parameter(Parameter &&) = delete;

  virtual void print(std::ostream &out, int indent = 0) const override;
};

class Prototype : public Expression {
  const std::string name_;
  const std::vector<std::unique_ptr<const Parameter>> params_;

public:
  Prototype(const std::string &name,
            std::vector<std::unique_ptr<const Parameter>> params)
      : name_(name), params_(std::move(params)){};
  Prototype(const Prototype &) = delete;
  Prototype(Prototype &&) = delete;

  virtual void print(std::ostream &out, int indent = 0) const override;
};

class Function : public Expression {
  std::unique_ptr<const Prototype> prototype_;
  const std::vector<std::unique_ptr<const Expression>> body_;

public:
  Function(std::unique_ptr<const Prototype> prototype,
           std::vector<std::unique_ptr<const Expression>> body)
      : prototype_(std::move(prototype)), body_(std::move(body)) {}
  Function(const Function &) = delete;
  Function(Function &&) = delete;

  virtual void print(std::ostream &out, int indent = 0) const override;
};

class Call : public Expression {
  const std::string name_;
  const std::vector<std::unique_ptr<const Expression>> args_;

public:
  Call(const std::string &name,
       std::vector<std::unique_ptr<const Expression>> args)
      : name_(name), args_(std::move(args)) {}
  Call(const Call &) = delete;
  Call(Call &&) = delete;

  virtual void print(std::ostream &out, int indent = 0) const override;
};

class Assignment : public Expression {
  std::unique_ptr<const Expression> left_;
  std::unique_ptr<const Expression> right_;

public:
  Assignment(std::unique_ptr<const Expression> left,
             std::unique_ptr<const Expression> right)
      : left_(std::move(left)), right_(std::move(right)) {}

  Assignment(const Assignment &) = delete;
  Assignment(Assignment &&) = delete;

  virtual void print(std::ostream &out, int indent = 0) const override;
};

class TupleAssignment : public Expression {
  const std::vector<std::unique_ptr<const Expression>> left_;
  const std::vector<std::unique_ptr<const Expression>> right_;

public:
  TupleAssignment(std::vector<std::unique_ptr<const Expression>> left,
                  std::vector<std::unique_ptr<const Expression>> right)
      : left_(std::move(left)), right_(std::move(right)) {}

  TupleAssignment(const TupleAssignment &) = delete;
  TupleAssignment(TupleAssignment &&) = delete;

  virtual void print(std::ostream &out, int indent = 0) const override;
};

} // namespace ast
} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_EXPRESSIONS_H
