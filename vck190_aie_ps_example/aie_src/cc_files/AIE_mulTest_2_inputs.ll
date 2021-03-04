; ModuleID = 'AIE_mulTest_2_inputs.cc'
source_filename = "AIE_mulTest_2_inputs.cc"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64"

; Function Attrs: noinline norecurse nounwind optsize
define dso_local void @_Z11AIE_mulTestPfS_S_(float* noalias nocapture readonly %A, float* noalias nocapture readonly %B, float* noalias nocapture %C) local_unnamed_addr #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata float* %A, metadata !14, metadata !DIExpression()), !dbg !20
  call void @llvm.dbg.value(metadata float* %B, metadata !15, metadata !DIExpression()), !dbg !21
  call void @llvm.dbg.value(metadata float* %C, metadata !16, metadata !DIExpression()), !dbg !22
  call void @llvm.dbg.value(metadata i32 0, metadata !17, metadata !DIExpression()), !dbg !23
  br label %vector.body, !dbg !24

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ], !dbg !25
  %0 = getelementptr inbounds float, float* %A, i64 %index, !dbg !27
  %1 = bitcast float* %0 to <8 x float>*, !dbg !27
  %wide.load = load <8 x float>, <8 x float>* %1, align 4, !dbg !27, !tbaa !28
  %2 = getelementptr inbounds float, float* %B, i64 %index, !dbg !32
  %3 = bitcast float* %2 to <8 x float>*, !dbg !32
  %wide.load11 = load <8 x float>, <8 x float>* %3, align 4, !dbg !32, !tbaa !28
  %4 = fmul fast <8 x float> %wide.load11, %wide.load, !dbg !33
  %5 = getelementptr inbounds float, float* %C, i64 %index, !dbg !34
  %6 = bitcast float* %5 to <8 x float>*, !dbg !35
  store <8 x float> %4, <8 x float>* %6, align 4, !dbg !35, !tbaa !28
  %index.next = add i64 %index, 8, !dbg !25
  %7 = icmp eq i64 %index, 0, !dbg !25
  br i1 %7, label %for.cond.cleanup, label %vector.body, !dbg !25, !llvm.loop !36

for.cond.cleanup:                                 ; preds = %vector.body
  ret void, !dbg !40
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline norecurse nounwind optsize "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 8.0.1 ", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "AIE_mulTest_2_inputs.cc", directory: "/net/shared/kafi/AIE_mulTest/cc_files")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 8.0.1 "}
!7 = distinct !DISubprogram(name: "AIE_mulTest", linkageName: "_Z11AIE_mulTestPfS_S_", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !13)
!8 = !DISubroutineType(types: !9)
!9 = !{null, !10, !10, !10}
!10 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !11)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!13 = !{!14, !15, !16, !17}
!14 = !DILocalVariable(name: "A", arg: 1, scope: !7, file: !1, line: 1, type: !10)
!15 = !DILocalVariable(name: "B", arg: 2, scope: !7, file: !1, line: 1, type: !10)
!16 = !DILocalVariable(name: "C", arg: 3, scope: !7, file: !1, line: 1, type: !10)
!17 = !DILocalVariable(name: "i", scope: !18, file: !1, line: 4, type: !19)
!18 = distinct !DILexicalBlock(scope: !7, file: !1, line: 4, column: 5)
!19 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!20 = !DILocation(line: 1, column: 39, scope: !7)
!21 = !DILocation(line: 1, column: 62, scope: !7)
!22 = !DILocation(line: 1, column: 86, scope: !7)
!23 = !DILocation(line: 4, column: 14, scope: !18)
!24 = !DILocation(line: 4, column: 5, scope: !18)
!25 = !DILocation(line: 4, column: 30, scope: !26)
!26 = distinct !DILexicalBlock(scope: !18, file: !1, line: 4, column: 5)
!27 = !DILocation(line: 5, column: 16, scope: !26)
!28 = !{!29, !29, i64 0}
!29 = !{!"float", !30, i64 0}
!30 = !{!"omnipotent char", !31, i64 0}
!31 = !{!"Simple C++ TBAA"}
!32 = !DILocation(line: 5, column: 21, scope: !26)
!33 = !DILocation(line: 5, column: 20, scope: !26)
!34 = !DILocation(line: 5, column: 9, scope: !26)
!35 = !DILocation(line: 5, column: 14, scope: !26)
!36 = distinct !{!36, !24, !37, !38, !39}
!37 = !DILocation(line: 5, column: 24, scope: !18)
!38 = !{!"llvm.loop.vectorize.enable", i1 true}
!39 = !{!"llvm.loop.isvectorized", i32 1}
!40 = !DILocation(line: 6, column: 1, scope: !7)
