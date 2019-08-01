//package sample.spdlog
//package test

import kotlinx.cinterop.*
import platform.posix.*
import platform.posix.memcpy
import test.*

fun main() {
    val name = "You"
    println("Hey $name")
    val theStruct = interpretPointed<TheStruct>(create().rawValue)
    theStruct.iPub = 21
    theStruct.foo(theStruct.ptr)
    MyStruct.create2()

//    val x = bar(null)
//    val i = x.iPub
//    testCtor()
    testCtor2()
    testCtor3()
    test2()
}

class MyStruct(rawPtr: NativePtr) : CStructVar(rawPtr) {

    companion object : Type(16, 8) {
        fun create2() : MyStruct {
            println ("Type::create")
            return interpretPointed<MyStruct>(create().rawValue)
        }
    }

    constructor() : this(create().rawValue) {
        iPub = 24
    }

    var iPub: Int
        get() = memberAt<IntVar>(8).value
        set(value) { memberAt<IntVar>(8).value = value }

//    init {
//        iPub = 24
//        memcpy(this.ptr, create(), typeOf<MyStruct>().size.convert())
//    }

    fun foo(arg1: CValuesRef<MyStruct>? = null): Int {
        memScoped {
            return kniBridge_0(rawPtr, arg1?.getPointer(memScope).rawValue)
        }
    }
    fun foo(i: Int): Unit {
        println("foo(i: Int = $i): Unit")
    }
}
@SymbolName("test_kniBridge0")
private external fun kniBridge_0(p0: NativePtr, p1: NativePtr): Int

fun testCtor() {
    println("testCtor")
    val cxx = nativeHeap.alloc<TheStruct>() {
        memcpy(ptr, create(), typeOf<TheStruct>().size.convert()) // use placement new here
    }
//    memcpy(cxx.ptr, create(), typeOf<TheStruct>().size.convert()) // use placement new here
    cxx.foo(null)
    nativeHeap.free(cxx)
}

fun testCtor2() {
    println("testCtor2: MyStruct(create().rawValue)")
    val xs = MyStruct(create().rawValue)
    memScoped {
        xs.foo()
    }
    xs.foo(xs.ptr)
    xs.foo(111)
}

fun testCtor3() {
    println("testCtor3: MyStruct()")
    val xs = MyStruct()
    xs.foo()
}

fun test2() {
    println("test2")
	val theS = bar(null)
//    val cStruct = cValue<TheStruct> {
//        iPub = 37
//    }
    memScoped {
        val aStruct = alloc<MyStruct>()
        memcpy(aStruct.ptr, create(), typeOf<TheStruct>().size.convert())

        aStruct.foo(null)
        aStruct.iPub = 37
        aStruct.foo()
    }


//    cStruct.foo(null)
    //    val aStruct = TheStruct(create().rawValue) // create().rawValue
//    println(aStruct?.pointed?.iPub) // OK
//    println(aStruct.iPub)
//    println(get42())
//    theS::class.members.forEach {it -> println(it.toString())}
//    val anS = TheStruct(create())
//	theS.foo(null)
	println("theS.useContents {iPub} = ${theS.useContents {iPub}}" )
}
