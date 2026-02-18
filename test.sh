#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" >tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 8 "a=3; b=5; return a + b;"
# if 文
assert 1 "if (1) return 1; return 0;"
assert 0 "if (0) return 1; return 0;"

# if-else 文
assert 1 "if (1) return 1; else return 0;"
assert 0 "if (0) return 1; else return 0;"

#for-while　文
assert 3 "i=0; while(i<3) i=i+1; return i;"
assert 3 "for(i=0;i<3;i=i+1); return i;"

echo OK
