#include "expressions.h"

namespace lang {
namespace compiler {
namespace ast {

void print_body(std::ostream &out, int indent,
                const std::vector<std::unique_ptr<const Expression>> &body) {
  if (body.empty()) {
    out << "\n" << std::string(indent + 4, ' ') << "())";
    return;
  }

  auto it = body.begin();
  out << "\n" << std::string(indent + 4, ' ') << '(';
  (*it)->print(out, indent + 4);

  for (++it; it != body.end(); ++it) {
    out << "\n" << std::string(indent + 5, ' ');
    (*it)->print(out, indent + 5);
  }
}

void Assignment::print(std::ostream &out, int indent) const {
  out << "(asgn ";
  if (left_ != nullptr) {
    out << "\n";
    left_->print(out, indent + 6);
  } else {
    out << "nil";
  }
  if (right_ != nullptr) {
    out << "\n";
    right_->print(out, indent + 6);
  } else {
    out << "nil";
  }
  out << ")";
}

void BinaryExpression::print(std::ostream &out, int indent) const {
  out << "(" << op_;
  out << "\n" << std::string(indent + 1, ' ');
  if (left_ != NULL) {
    left_->print(out, indent + 1);
  } else {
    out << "nil";
  }
  out << "\n" << std::string(indent + 1, ' ');
  if (right_ != NULL) {
    right_->print(out, indent + 1);
  } else {
    out << "nil";
  }
  out << ")";
}

void Call::print(std::ostream &out, int indent) const {
  out << "(call " << name_;

  if (args_.empty()) {
    out << ")";
    return;
  }

  auto it = args_.begin();
  out << "\n" << std::string(indent + 7, ' ');
  (*it)->print(out, indent + 7);

  for (++it; it != args_.end(); ++it) {
    out << "\n" << std::string(indent + 7, ' ');
    (*it)->print(out, indent + 7);
  }
  out << ")";
}

void Function::print(std::ostream &out, int indent) const {
  out << "(fn ";
  prototype_->print(out, indent + 4);
  print_body(out, indent, body_);
  out << "))";
}

void If::print(std::ostream &out, int indent) const {
  out << "(if ";
  cond_->print(out, indent + 4);

  print_body(out, indent, then_);
  print_body(out, indent, else_);

  out << ")";
}

void Identifier::print(std::ostream &out, int indent) const {
  out << "(id " << name_ << ")";
}

void Integer::print(std::ostream &out, int indent) const {
  out << "(int " << value_ << ")";
}

void TupleAssignment::print(std::ostream &out, int indent) const {
  out << "(asgn ";
  auto lit = left_.begin();
  (*lit)->print(out);
  auto rit = right_.begin();
  (*rit)->print(out);

  for (++lit, ++rit; lit != left_.end() && rit != right_.end(); ++lit, ++rit) {
    (*lit)->print(out);
    (*rit)->print(out);
  }
}

void Parameter::print(std::ostream &out, int indent) const {
  out << "(param " << (constant_ ? "val" : "var") << " " << name_ << ")";
}

void Prototype::print(std::ostream &out, int indent) const {
  out << "(proto " << name_;

  if (params_.empty()) {
    out << " ())";
    return;
  }

  auto it = params_.begin();
  out << "\n" << std::string(indent + 7, ' ') << '(';
  (*it)->print(out);

  for (++it; it != params_.end(); ++it) {
    out << "\n";
    out << std::string(indent + 8, ' ');
    (*it)->print(out, indent + 8);
  }
  out << "))";
}

void Value::print(std::ostream &out, int indent) const {
  out << "(" << (constant_ ? "val" : "var") << " " << name_;
  if (value_ == nullptr) {
    out << " nil";
  } else {
    out << "\n" << std::string(indent + 6, ' ');
    value_->print(out, indent + 6);
  }
  out << ")";
}

} // namespace ast
} // namespace compiler
} // namespace lang
