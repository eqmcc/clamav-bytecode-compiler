// RUN: %clang_cc1 -fsyntax-only -verify %s

extern void foo();

@protocol MyProtocol @end

@interface MyClass @end

int main()
{
  MyClass <MyProtocol> *obj_p;
  MyClass *obj_cp;

  obj_cp = obj_p;  
  obj_p = obj_cp;

  if (obj_cp == obj_p)
    foo();

  if (obj_p == obj_cp)
    foo();

}


