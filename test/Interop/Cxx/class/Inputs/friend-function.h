#pragma once

struct A {
  void memberInA(int x) const;
};

struct B {
  // This should not get imported as a member of B, because friendship means
  // nothing in Swift
  friend void A::memberInA(int) const;

  void memberInB() const;
};

// C should not mistakenly "inherit" the friend declarations in B
struct C : B {
  void memberInC() const;
};
