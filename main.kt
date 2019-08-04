//package sample.spdlog
//package test

import kotlinx.cinterop.*
import platform.posix.*
import platform.posix.memcpy
import test.*

fun main() {
    val name = "You"
    println("Hey $name")
    val theStruct = interpretPointed<ns__TheStruct>(ns__create().rawValue)
    theStruct.iPub = 21
    theStruct.foo(theStruct.ptr)

    test0();

    testCtor2()
//    testCtor3()
    test2()
}

fun testCtor() {
    println("testCtor")
    val cxx = nativeHeap.alloc<ns__TheStruct>() {
        memcpy(ptr, ns__create(), typeOf<ns__TheStruct>().size.convert()) // use placement new here
    }
//    memcpy(cxx.ptr, ns__create(), typeOf<TheStruct>().size.convert()) // use placement new here
    cxx.foo(null)
    nativeHeap.free(cxx)
}

fun testCtor2() {
    println("testCtor2: MyStruct(ns__create().rawValue)")
    val xs = ns__TheStruct(ns__create().rawValue)
    memScoped {
        xs.foo(null)
    }
    xs.foo(xs.ptr)
}
/*
fun testCtor3() {
    println("testCtor3: MyStruct()")
    val xs = MyStruct()
    xs.foo()
}
*/
fun test0() {
    memScoped {
        val aStruct = alloc<ns__NoName>()
        aStruct.noNameMember()
    }
}

fun test2() {
    println("test2")
	val x = ns__bar(null)
//    val theS = interpretPointed<ns__TheStruct>(ns__bar(null).rawValue)
//    theS.foo(null)

	println("x.useContents {iPub} = ${x.useContents {iPub}}" )
}
