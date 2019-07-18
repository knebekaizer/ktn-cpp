0. parse AST
0. Visit recursively, collect supported entries
0. For each entity in list generate wrappers

The point is: use Kotlin/C binding only, while C wrapper impl to be compiled and linked by c++ compiler 

Wrappers are:

extern "C" { ... }
- wrapper funtions for free C++ functions and member functions, e.g.
- fake forward declarations to keep type checks (C++ type tracking):


[[file://x.h]]
[[file://x.cpp]]

kotlin:
- Conveniency classes to wrap ugly C binding into class methods

### Marshalling (params and return value)

By CXTypeKind: primitive, POD, non-POD (opaque object), pointer (all 3 types)

Declaration:

	[const] ReturnType_Mangled functionName_Mangled ( 
		[ [const] ReceiverType_Mangled * self, ]
		ArgType_Mangled arg1 [, ...] );
	
Definition:

	[const] ReturnType_Mangled functionName_Mangled ( 
		[ [const] ReceiverType_Mangled * self, ]
		ArgType_Mangled arg1 [, ...] ) {
			return (ReturnType_Mangled) ([const] ReceiverType *)self->(arg1, ...);
		}

Creator aka ctor:

	ObjectType_Mangled * createN ( argsListMangled ) {
		return new ObjectType( argList );
	}
	
	TODO: default constructors (implicitly generated?)

Deleter aka dtor:

	ObjectType_Mangled_delete (ObjectType_Mangled * that) {
		delete that;
	}
	
	TODO: delete []
	
Callbacks:

	TODO:
	class ObjectType_ImplKotlin : public : ObjectType {
		// TODO ctors ???
		ReturnType methodName(argList) [const] {
			return methodName_Mangled(this, argList); // use C binding to Kotlin
		}
	};


- typedef: use typedef for pretty print, but match by CanonicalType

### Type MMM (matching, mangling and mapping):

- One-way only: from real type to mangled name, not vice versa. Can do it on the fly using text representation only (type name)

Map types as:
- primitive - primitive, canonical
- pointer to primitive - the same as Kotlin/C
- pointer to __any__ object (POD or non-POD - pointer to mangled struct name, forward decl, inclomplete type)
- const pointer
- POD - blob, using memcopy
- non-POD - pointer to blob in heap, using actual ctors and dtor

Below: cType is the same primitive type or mangled name otherwise
	CXTypeKind
		isPrimitiveType ? asNativeType
		                : isPointer ? isPointeePriimtive ? asNativeType
		                                                : mangling(type)
		                           : asBlobMangled