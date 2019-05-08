#ifndef LANG_COMPILER_EXPRESSIONS_H
#define LANG_COMPILER_EXPRESSIONS_H

#include <memory>
#include <ostream>
#include <vector>

#define MAKE_VISITABLE                                                         \
  virtual void accept(Visitor &v) const override { v.visit(getptr()); }

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

  virtual void print(std::ostream &out, int indent = 0) const = 0;

  virtual void accept(Visitor &) const = 0;

  friend std::ostream &operator<<(std::ostream &out, const Expression &node) {
    node.print(out);
    return out;
  }
};

typedef std::vector<std::shared_ptr<const Expression>> Expressions;

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
  virtual void visit(std::shared_ptr<const Assignment>) = 0;
  virtual void visit(std::shared_ptr<const BinaryExpression>) = 0;
  virtual void visit(std::shared_ptr<const Call>) = 0;
  virtual void visit(std::shared_ptr<const Function>) = 0;
  virtual void visit(std::shared_ptr<const If>) = 0;
  virtual void visit(std::shared_ptr<const Identifier>) = 0;
  virtual void visit(std::shared_ptr<const Integer>) = 0;
  virtual void visit(std::shared_ptr<const Parameter>) = 0;
  virtual void visit(std::shared_ptr<const Prototype>) = 0;
  virtual void visit(std::shared_ptr<const TupleAssignment>) = 0;
  virtual void visit(std::shared_ptr<const Value>) = 0;
};

class NoopVisitor : public Visitor {
  void visit(std::shared_ptr<const Assignment>) {}
  void visit(std::shared_ptr<const BinaryExpression>) {}
  void visit(std::shared_ptr<const Call>) {}
  void visit(std::shared_ptr<const Function>) {}
  void visit(std::shared_ptr<const If>) {}
  void visit(std::shared_ptr<const Identifier>) {}
  void visit(std::shared_ptr<const Integer>) {}
  void visit(std::shared_ptr<const Parameter>) {}
  void visit(std::shared_ptr<const Prototype>) {}
  void visit(std::shared_ptr<const TupleAssignment>) {}
  void visit(std::shared_ptr<const Value>) {}
};

class Assignment : public Expression,
                   public std::enable_shared_from_this<Assignment> {
  std::shared_ptr<const Expression> left_;
  std::shared_ptr<const Expression> right_;

public:
  Assignment(std::shared_ptr<const Expression> left,
             std::shared_ptr<const Expression> right)
      : left_(std::move(left)), right_(std::move(right)) {}

  Assignment(const Assignment &) = delete;
  Assignment(Assignment &&) = delete;

  std::shared_ptr<const Assignment> getptr() const {
    return shared_from_this();
  }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class BinaryExpression : public Expression,
                         public std::enable_shared_from_this<BinaryExpression> {
  const char op_;
  std::shared_ptr<const Expression> left_, right_;

public:
  BinaryExpression(char op, std::shared_ptr<const Expression> left,
                   std::shared_ptr<const Expression> right)
      : op_(op), left_(std::move(left)), right_(std::move(right)) {}
  BinaryExpression(const BinaryExpression &) = delete;
  BinaryExpression(BinaryExpression &&) = delete;

  std::shared_ptr<BinaryExpression const> getptr() const {
    return shared_from_this();
  }

  char op() const { return op_; }
  const Expression &right() const { return *right_; }
  const Expression &left() const { return *left_; }
  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class Call : public Expression, public std::enable_shared_from_this<Call> {
  const std::string name_;
  const Expressions args_;

public:
  Call(const std::string &name, Expressions args)
      : name_(name), args_(std::move(args)) {}
  Call(const Call &) = delete;
  Call(Call &&) = delete;

  std::shared_ptr<Call const> getptr() const { return shared_from_this(); }

  const std::string &name() const { return name_; }
  const Expressions &args() const { return args_; }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class Function : public std::enable_shared_from_this<Function>,
                 public Expression {
  std::shared_ptr<const Prototype> prototype_;
  const Expressions body_;

public:
  Function(std::shared_ptr<const Prototype> prototype, Expressions body)
      : prototype_(std::move(prototype)), body_(std::move(body)) {}
  Function(const Function &) = delete;
  Function(Function &&) = delete;

  std::shared_ptr<Function const> getptr() const { return shared_from_this(); }

  const Prototype &proto() const { return *prototype_; }
  const Expressions &body() const { return body_; }

  virtual void print(std::ostream &out, int indent = 0) const override;

  MAKE_VISITABLE;
};

class If : public Expression, public std::enable_shared_from_this<If> {
  std::shared_ptr<const Expression> cond_;
  const Expressions then_;
  const Expressions else_;

public:
  If(std::shared_ptr<const Expression> cond, Expressions thn, Expressions els)
      : cond_(std::move(cond)), then_(std::move(thn)), else_(std::move(els)) {}
  If(const If &) = delete;
  If(If &&) = delete;

  std::shared_ptr<If const> getptr() const { return shared_from_this(); }

  const Expression &cond() const { return *cond_; }
  const Expressions &thn() const { return then_; }
  const Expressions &els() const { return else_; }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class Identifier : public Expression,
                   public std::enable_shared_from_this<Identifier> {
  const std::string name_;

public:
  Identifier(const std::string &name) : name_(name) {}
  Identifier(const Identifier &) = delete;
  Identifier(Identifier &&) = delete;

  std::shared_ptr<Identifier const> getptr() const {
    return shared_from_this();
  }

  const std::string &name() const { return name_; }

  void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class Integer : public Expression,
                public std::enable_shared_from_this<Integer> {
  const long value_;

public:
  Integer(long value) : value_(value) {}
  Integer(const Integer &) = delete;
  Integer(Integer &&) = delete;

  std::shared_ptr<Integer const> getptr() const { return shared_from_this(); }

  long value() const { return value_; }

  void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class Prototype : public Expression,
                  public std::enable_shared_from_this<Prototype> {
  const std::string name_;
  const std::vector<std::shared_ptr<const Parameter>> params_;

public:
  Prototype(const std::string &name,
            std::vector<std::shared_ptr<const Parameter>> params)
      : name_(name), params_(std::move(params)){};
  Prototype(const Prototype &) = delete;
  Prototype(Prototype &&) = delete;

  std::shared_ptr<Prototype const> getptr() const { return shared_from_this(); }

  const std::string &name() const { return name_; }
  const std::vector<std::shared_ptr<const Parameter>> &params() const {
    return params_;
  }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class TupleAssignment : public Expression,
                        public std::enable_shared_from_this<TupleAssignment> {
  const Expressions left_;
  const Expressions right_;

public:
  TupleAssignment(Expressions left, Expressions right)
      : left_(std::move(left)), right_(std::move(right)) {}

  TupleAssignment(const TupleAssignment &) = delete;
  TupleAssignment(TupleAssignment &&) = delete;

  std::shared_ptr<TupleAssignment const> getptr() const {
    return shared_from_this();
  }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class BaseValue : public Expression {
protected:
  const bool constant_;
  const std::string name_;

  std::shared_ptr<const Expression> value_;

public:
  BaseValue(bool constant, const std::string &name)
      : constant_(constant), name_(name), value_(nullptr) {}
  BaseValue(bool constant, const std::string &name,
            std::shared_ptr<const Expression> value)
      : constant_(constant), name_(name), value_(std::move(value)) {}
  BaseValue(const Value &) = delete;
  BaseValue(Value &&) = delete;

  virtual ~BaseValue() = default;

  bool constant() const { return constant_; }
  const std::string &name() const { return name_; }
  const Expression &value() const { return *value_; }
};

class Value : public BaseValue, public std::enable_shared_from_this<Value> {
public:
  Value(bool constant, const std::string &name) : BaseValue(constant, name) {}
  Value(bool constant, const std::string &name,
        std::shared_ptr<const Expression> value)
      : BaseValue(constant, name, std::move(value)) {}
  Value(const Value &) = delete;
  Value(Value &&) = delete;

  std::shared_ptr<Value const> getptr() const { return shared_from_this(); }

  bool constant() const { return constant_; }
  const std::string &name() const { return name_; }
  const Expression &value() const { return *value_; }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

class Parameter : public BaseValue,
                  public std::enable_shared_from_this<Parameter> {
public:
  Parameter(bool constant, const std::string &name)
      : BaseValue(constant, name) {}
  Parameter(const Parameter &) = delete;
  Parameter(Parameter &&) = delete;

  std::shared_ptr<Parameter const> getptr() const { return shared_from_this(); }

  virtual void print(std::ostream &out, int indent = 0) const override;
  MAKE_VISITABLE;
};

} // namespace ast

namespace cfg {

class BasicBlock {
  std::vector<std::shared_ptr<const ast::Expression>> _expressions;
  std::vector<std::unique_ptr<const BasicBlock>> _exits;

public:
  BasicBlock() {}
  BasicBlock(const BasicBlock &) = delete;
  BasicBlock(BasicBlock &&) = delete;

  bool empty() const;
  void emplace_back(std::shared_ptr<const ast::Expression>);

  void print(std::ostream &out, int indent = 0) const;

  friend std::ostream &operator<<(std::ostream &out, const BasicBlock &block) {
    block.print(out);
    return out;
  }
};

} // namespace cfg
} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_EXPRESSIONS_H
