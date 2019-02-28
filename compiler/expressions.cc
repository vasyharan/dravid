#include "expressions.h"

namespace lang {
namespace compiler {
namespace ast {

void Assign::print(std::ostream &out, int indent) const {
  out << std::string(indent, ' ') << "(asgn ";
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
  out << std::string(indent, ' ') << "(" << op_;
  if (left_ != NULL) {
    out << "\n";
    left_->print(out, indent + 1);
  } else {
    out << " nil";
  }
  if (right_ != NULL) {
    out << "\n";
    right_->print(out, indent + 1);
  } else {
    out << " nil";
  }
  out << ")";
}

void Function::print(std::ostream &out, int indent) const {
  out << "(fn ";
  prototype_->print(out, indent + 4);

  if (body_.empty()) {
    out << "\n" << std::string(indent + 4, ' ') << "())";
    return;
  }

  auto it = body_.begin();
  out << "\n" << std::string(indent + 4, ' ') << '(';
  (*it)->print(out);

  for (++it; it != body_.end(); ++it) {
    out << "\n";
    (*it)->print(out, indent + 5);
  }
  out << "))";
}

void Identifier::print(std::ostream &out, int indent) const {
  out << std::string(indent, ' ') << "(id " << name_ << ")";
}

void Integer::print(std::ostream &out, int indent) const {
  out << std::string(indent, ' ');
  out << "(int " << value_ << ")";
}

void Value::print(std::ostream &out, int indent) const {
  out << std::string(indent, ' ');
  out << "(param " << (constant_ ? "val" : "var") << " " << name_;
  // if (hasDefault) {
  //   if (defValue == nullptr) {
  //     out << " nil";
  //   } else {
  //     out << "\n";
  //     defValue->print(out, indent + 1);
  //   }
  // }
  out << ")";
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
    (*it)->print(out, indent + 8);
  }
  out << "))";
}

} // namespace ast
} // namespace compiler
} // namespace lang
