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

assert 3 "int main(){ return 3; }"

assert 8 "int main(){ int a=3; int b=5; return a + b; }"

assert 1 "int main(){ if (1) return 1; return 0; }"
assert 0 "int main(){ if (0) return 1; return 0; }"

assert 1 "int main(){ if (1) return 1; else return 0; }"
assert 0 "int main(){ if (0) return 1; else return 0; }"

assert 3 "int main(){ i=0; while(i<3) i=i+1; return i; }"
assert 3 "int main(){ for(i=0;i<3;i=i+1); return i; }"

echo OK
