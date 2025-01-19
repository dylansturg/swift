// REQUIRES: swift_swift_parser

// RUN: %target-swift-frontend %s -swift-version 5 -module-name main -disable-availability-checking -typecheck -plugin-path %swift-plugin-dir -dump-macro-expansions 2>&1 | %FileCheck --match-full-lines %s

@_SwiftifyImport(.sizedBy(pointer: .param(1), size: "size"))
func myFunc(_ ptr: UnsafeRawPointer?, _ size: CInt) {
}

// CHECK:      @_alwaysEmitIntoClient
// CHECK-NEXT: func myFunc(_ ptr: UnsafeRawBufferPointer?) {
// CHECK-NEXT:     return myFunc(ptr?.baseAddress, CInt(exactly: ptr?.count ?? 0)!)
// CHECK-NEXT: }
