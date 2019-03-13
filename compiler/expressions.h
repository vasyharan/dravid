#ifndef LANG_COMPILER_EXPRESSIONS_H
#define LANG_COMPILER_EXPRESSIONS_H

#include <memory>
#include <ostream>
#include <vector>

#define MAKE_VISITABLE                                                         \
  virtual void accept(Visitor &v) const override { v.visit(*this); }

namespace lang {
namespace compiler {
namespace ast {

class Visitor;

class Expression {
public:
  Expression() = default;
  Expression(const Expression &) = delete;
  Expression(Expression &&) = delete;

  virtual ~Expression() = default;

  // virtual void accept(Visitor &visitor) const = 0;
  virtual void print(std::ostream &out, int indent = 0) const = 0;
  virtual void accept(Visitor &) const = 0;

  friend std::ostream &operator<<(std::ostream &out, const Expression &node) {
    node.print(out);
    return out;
  }
};

class Assignment;
class BinaryExpression;
class Call;
class Function;
class If;
class Identifier;
class Integer;
class Parameter;
class Prototype;
class TupleAssignment;
class Value;

class Visitor {
public:
  virtual void visit(const Assignment &) = 0;
  virtual void visit(const BinaryExpression &) = 0;
  virtual void visit(const Call &) = 0;
  virtual void visit(const Function &) = 0;
  virtual void visit(const If &) = 0;
  virtual void visit(const Identifier &) = 0;
  virtual void visit(const Integer &) = 0;
  virtual void visit(const Parameter &) = 0;
  virtual void visit(const Prototype &) = 0;
  virtual void visit(const TupleAssignment &) = 0;
  virtual void visit(const Value &) = 0;
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
  MAKE_VISITABLE;
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

  char op() const { return op_; }
  const Expression &right() const { return *right_; }
  const Expression &left() const { return *left_; }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
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

  const std::string &name() const { return name_; }
  const std::vector<std::unique_ptr<const Expression>> &args() const {
    return args_;
  }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
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

  const Prototype &proto() const { return *prototype_; }
  const std::vector<std::unique_ptr<const Expression>> &body() const {
    return body_;
  }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class If : public Expression {
  std::unique_ptr<const Expression> cond_;
  const std::vector<std::unique_ptr<const Expression>> then_;
  const std::vector<std::unique_ptr<const Expression>> else_;

public:
  If(std::unique_ptr<const Expression> cond,
     std::vector<std::unique_ptr<const Expression>> thn,
     std::vector<std::unique_ptr<const Expression>> els)
      : cond_(std::move(cond)), then_(std::move(thn)), else_(std::move(els)) {}
  If(const If &) = delete;
  If(If &&) = delete;

  const Expression &cond() const { return *cond_; }
  const std::vector<std::unique_ptr<const Expression>> &thn() const {
    return then_;
  }
  const std::vector<std::unique_ptr<const Expression>> &els() const {
    return else_;
  }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class Identifier : public Expression {
  const std::string name_;

public:
  Identifier(const std::string &name) : name_(name) {}
  Identifier(const Identifier &) = delete;
  Identifier(Identifier &&) = delete;

  const std::string &name() const { return name_; }
  void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class Integer : public Expression {
  const long value_;

public:
  Integer(long value) : value_(value) {}
  Integer(const Integer &) = delete;
  Integer(Integer &&) = delete;

  long value() const { return value_; }
  void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
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

  const std::string &name() const { return name_; }
  const std::vector<std::unique_ptr<const Parameter>> &params() const {
    return params_;
  }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
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
  MAKE_VISITABLE;
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

  bool constant() const { return constant_; }
  const std::string &name() const { return name_; }
  const Expression &value() const { return *value_; }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class Parameter : public Value {
public:
  Parameter(bool constant, const std::string &name) : Value(constant, name) {}
  Parameter(const Parameter &) = delete;
  Parameter(Parameter &&) = delete;

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

} // namespace ast
} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_EXPRESSIONS_H
