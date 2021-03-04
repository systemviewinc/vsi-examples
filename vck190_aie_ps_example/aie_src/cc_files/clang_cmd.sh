~/projects/llvm/llvm-exec/bin/clang -I ~/projects/llvm/llvm-exec/include --target=aarch64 -finline-hint-functions -ffast-math -fvectorize -Rpass=loop-vectorize -Rpass-analysis=loop-vectorize -Rpass-missed=loop-vectorize -fslp-vectorize -S -emit-llvm -mllvm -force-vector-width=8 -c -g -I ./include $1 $2.cc
~/projects/llvm/llvm-exec/bin/vsi-aiebe --top-function $3 $2.ll -o $2.cpp
