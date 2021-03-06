Templates
=========

Loci implements templates, to provide parametric polymorphism, one use of which is to define types of collections. The syntax is extremely similar to C++:

.. code-block:: c++

	template <typename T>
	class Class(T v) { }

Note that 'typename' is the only valid specifier for a type; 'class' is not valid in Loci.

Type Properties
---------------

Loci allows type properties to be expressed through the use of :doc:`Predicates <Predicates>`. For example:

.. code-block:: c++

	template <typename T>
	interface Comparable {
		compare_result_t compare(const T& v) const;
	}
	
	template <typename T: Comparable<T>>
	class Class(T v0, T v1) {
		static Create(T v0, T v1) {
			return @(move v0, move v1);
		}
		
		bool lessThan() const {
			return @v0 < @v1;
		}
	}

The interface, which is itself templated, requires that the parameter type 'T' to class 'Class' has the comparison operator specified. Note that constructors specified in interfaces can only be used in the context of template type parameters; it is meaningless (and will cause a compile-time error) to attempt to construct an interface itself outside of a template.

Since :doc:`primitive types are objects <PrimitiveObjects>`, even with their implementation being identical to C, they can also be used as template parameters:

.. code-block:: c++

	void function(){
		auto cInt = Class<int>(1, 2);
		assert cInt.lessThan();
	}

Function Templates
------------------

Loci also supports templated functions, e.g.:

.. code-block:: c++

	template <typename T>
	require(T : movable and T : comparable<T>)
	T getMin(T first, T second) {
		return first < second ?
			move first :
			move second;
	}

Similarly, it's possible to use templates with methods:

.. code-block:: c++

	template <typename T : movable>
	interface CastFromTestClass {
		static T castFromTestClass(const TestClass& object);
	}
	
	class TestClass() {
		// Etc.
		
		template <typename T>
		T cast() const require(T : movable and T : CastFromTestClass<T>) {
			return T.castFromTestClass(self);
		}
		
		// Etc.
	}

Template Generation
-------------------

The above code shows the creation of both a class and a function that have a type parameter 'T'. In C++, instances of this constructs would be generated by the compiler for each type given as type 'T'. So the following C++ code would generate four different classes in the object code produced by the compiler:

.. code-block:: c++

	// This is C++ code.
	Class<int> cInt;
	Class<float> cFloat;
	
	struct StructType{ int a; float b; };
	Class<StructType> cS;
	
	Class<StructType *> cSPtr;

In Loci the compiler only creates one instance of the class, which works correctly for all possible types for T. This means that it's possible to define APIs in terms of templated types or functions **without** also providing the implementation. So, for example:

.. code-block:: c++

	// Module 'A'.
	export A 1.0.0 {
		
		template <typename T>
		class ExampleClass(T value) {
			static create = default;
		}
		
	}

.. code-block:: c++

	// Module 'B'.
	import A 1.0.0 {
		
		template <typename T>
		move(T : movable) // Type is only movable if T is movable.
		class ExampleClass {
			static ExampleClass<T> create(T value);
		}
		
	}

The advantages of this include:

* Faster compile times
* API compatibility despite implementation changes
* Smaller code

These are all the advantages of non-templated functions and classes, seamlessly extended to templated constructs. This is something that *can't be achieved* in C++, despite attempts to resolve this, such as C++'s *export* keyword.

The implementation essentially performs auto-boxing for primitive types, without needing to perform a heap allocation. However if the templated implementation is available to the compiler then it can easily optimise these uses and this can also be performed as part of link-time optimisation. As an example, LOCIC will generate LLVM IR that can be optimised to inline operations, and this can be performed even after linking modules together. See :doc:`Template Generators <TemplateGenerators>` for more information.

Hence there need be only be a runtime cost for templated constructs if:

* The definition and use are in separate modules.
* The modules have not been optimised after being linked together.

