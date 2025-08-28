/**
 * @name Custom Security Queries for Aether
 * @description Advanced security analysis specific to Aether codebase
 * @kind problem
 * @problem.severity warning
 * @security-severity 8.0
 * @precision high
 * @id cpp/aether-security-check
 * @tags security
 *       correctness
 *       external/cwe/cwe-119
 *       external/cwe/cwe-120
 *       external/cwe/cwe-125
 *       external/cwe/cwe-787
 */

import cpp

// Check for unsafe memory operations
class UnsafeMemoryFunction extends Function {
  UnsafeMemoryFunction() {
    this.getName() in [
      "memcpy", "memmove", "memset", "strcpy", "strncpy", "strcat", "strncat",
      "sprintf", "snprintf", "vsprintf", "vsnprintf", "gets", "scanf"
    ]
  }
}

// Check for buffer operations without proper bounds checking
predicate hasBufferOperation(FunctionCall fc) {
  fc.getTarget() instanceof UnsafeMemoryFunction
}

// Check for array access without bounds checking
predicate hasUncheckedArrayAccess(ArrayExpr ae) {
  not exists(IfStmt guard |
    guard.getCondition().getAChild*() = ae.getArrayOffset() and
    guard.getThen().getAChild*() = ae
  )
}

// Check for integer overflow in array indexing
predicate hasIntegerOverflow(ArrayExpr ae) {
  exists(AddExpr add | add = ae.getArrayOffset() |
    not exists(Expr check | 
      check.getParent*() instanceof IfStmt and
      check.toString().matches("%overflow%")
    )
  )
}

// Check for use-after-free patterns
predicate hasUseAfterFree(Variable v) {
  exists(FunctionCall fc1, FunctionCall fc2, VariableAccess va |
    fc1.getTarget().getName() in ["free", "delete"] and
    fc1.getAnArgument() = va and
    va.getTarget() = v and
    fc2.getAnArgument().(VariableAccess).getTarget() = v and
    fc2.getLocation().getStartLine() > fc1.getLocation().getStartLine()
  )
}

// Check for double-free vulnerabilities
predicate hasDoubleFree(Variable v) {
  exists(FunctionCall fc1, FunctionCall fc2, VariableAccess va1, VariableAccess va2 |
    fc1.getTarget().getName() in ["free", "delete"] and
    fc2.getTarget().getName() in ["free", "delete"] and
    fc1.getAnArgument() = va1 and
    fc2.getAnArgument() = va2 and
    va1.getTarget() = v and
    va2.getTarget() = v and
    fc1 != fc2
  )
}

// Check for format string vulnerabilities
predicate hasFormatStringVuln(FunctionCall fc) {
  fc.getTarget().getName() in ["printf", "sprintf", "snprintf", "fprintf"] and
  exists(Expr format | format = fc.getArgument(0) |
    not format instanceof StringLiteral
  )
}

// Check for weak cryptographic algorithms
predicate hasWeakCrypto(FunctionCall fc) {
  fc.getTarget().getName().toLowerCase().matches([
    "%md5%", "%sha1%", "%des%", "%rc4%", "%crc%"
  ]) and
  not fc.getTarget().getName().toLowerCase().matches([
    "%sha256%", "%sha512%", "%aes%", "%chacha%"
  ])
}

// Check for hardcoded credentials
predicate hasHardcodedCredential(StringLiteral sl) {
  sl.getValue().toLowerCase().matches([
    "%password%", "%secret%", "%key%", "%token%", "%credential%"
  ]) and
  sl.getValue().length() > 8
}

// Check for unsafe random number generation
predicate hasWeakRandom(FunctionCall fc) {
  fc.getTarget().getName() in ["rand", "srand", "random", "srandom"] and
  not exists(FunctionCall secure |
    secure.getTarget().getName().matches([
      "%secure%", "%crypto%", "%random_device%"
    ])
  )
}

// Main query combining all security checks
from Expr e, string message
where
  (
    e instanceof FunctionCall and hasBufferOperation(e) and
    message = "Potentially unsafe memory operation: " + e.toString()
  ) or (
    e instanceof ArrayExpr and hasUncheckedArrayAccess(e) and
    message = "Array access without bounds checking: " + e.toString()
  ) or (
    e instanceof ArrayExpr and hasIntegerOverflow(e) and
    message = "Potential integer overflow in array indexing: " + e.toString()
  ) or (
    exists(Variable v | hasUseAfterFree(v) and e.(VariableAccess).getTarget() = v) and
    message = "Potential use-after-free: " + e.toString()
  ) or (
    exists(Variable v | hasDoubleFree(v) and e.(VariableAccess).getTarget() = v) and
    message = "Potential double-free: " + e.toString()
  ) or (
    e instanceof FunctionCall and hasFormatStringVuln(e) and
    message = "Format string vulnerability: " + e.toString()
  ) or (
    e instanceof FunctionCall and hasWeakCrypto(e) and
    message = "Weak cryptographic algorithm: " + e.toString()
  ) or (
    e instanceof StringLiteral and hasHardcodedCredential(e) and
    message = "Hardcoded credential detected: " + e.toString()
  ) or (
    e instanceof FunctionCall and hasWeakRandom(e) and
    message = "Weak random number generation: " + e.toString()
  )
select e, message