Overview
========

Fit is a header-only C++11/C++14 library that provides utilities for functions and function objects. 

Fit is:

- Modern: Fit takes advantages of modern C++11/C++14 features. It support both `constexpr` initialization and `constexpr` evaluation of functions. It takes advantage of type deduction, varidiac templates, and perfect forwarding to provide a simple and modern interface. 
- Relevant: Fit provides utilities for functions and does not try to implement a functional language in C++. As such, Fit solves many problems relevant to C++ programmers, including initialization of function objects and lambdas, overloading with ordering, improved return type deduction, and much more.
- Lightweight: Fit builds simple lightweight abstraction on top of function objects. It does not require subscribing to an entire framework. Just use the parts you need.

Fit is divided into three components:

* Function Adaptors and Decorators: These enhance functions with additional capability.
* Functions: These return functions that achieve a specific purpose.
* Utilities: These are general utilities that are useful when defining or using functions

Github: [http://github.com/pfultz2/Fit](http://github.com/pfultz2/Fit)

Documentation: [http://fit.readthedocs.org](http://fit.readthedocs.org)
Quick Start
===========

Function Objects
----------------

In C++, a function object is just a class that overrides the call operator like this:

    // A sum function object
    struct sum_f
    {
        template<class T, class U>
        auto operator()(T x, U y) const
        {
            return x + y;
        }
    };

There are few things to note about this. First, the call operator member function is always declared `const`, which is generally required to be used with Fit.(Note: The [`mutable_`](mutable.md) adaptor can be used to make a mutable function object have a `const` call operator, but this should generally be avoided). Secondly, the `sum_f` class must be constructed first before it can be called:

    auto three = sum_f()(1, 2);

We can make it behave like a regular function if we construct the class as a global variable. The Fit library provides [`FIT_STATIC_FUNCTION`](function.md) to properly initialize the the function object at compile-time to avoid the [static initialization order fiasco](https://isocpp.org/wiki/faq/ctors#static-init-order) and possible ODR violations:

    FIT_STATIC_FUNCTION(sum) = sum_f();

Adaptors
--------

Now we have defined the function as a function object, we can add new "enhancements" to the function. We could make the function pipable using the [`pipable`](pipable.md) adaptor:

    FIT_STATIC_FUNCTION(sum) = pipable_adaptor<sum_f>();

This allows the parameters to piped into it, like this:

    auto three = 1 | sum(2);

Or we could make it an infix named operator as well using the [`infix`](infix.md) adaptor:

    FIT_STATIC_FUNCTION(sum) = infix_adaptor<sum_f>();

And it could be called like this:

    auto three = 1 <sum> 2;

Additionally each of the adaptors have a corresponding function version without the `_adaptor` suffix. So we could pass `sum` as a variable to the adaptors to make new functions. So we can do things like partial application and function composition if we wanted to:

    auto add_1 = partial(sum)(1);
    auto add_2 = compose(add_1, add_1);
    auto three = add_2(1);

Lambdas
-------

Instead of writing function objects which can be a little verbose, we can write the functions as lambdas instead. However, by default lambdas cannot be statically initialized at compile time. So we can use the [`FIT_STATIC_LAMBDA`](lambda.md) to initialize them at compile time:

    FIT_STATIC_FUNCTION(sum) = FIT_STATIC_LAMBDA(auto x, auto y)
    {
        return x + y;
    };

And we can apply the same adaptors as well:

    // Pipable sum
    FIT_STATIC_FUNCTION(sum) = pipable(FIT_STATIC_LAMBDA(auto x, auto y)
    {
        return x + y;
    });

We can also use [`FIT_STATIC_LAMBDA_FUNCTION`](lambda.md) so we dont have to repeat [`FIT_STATIC_LAMBDA`](lambda.md) for adaptors, and it can help avoid possible ODR violations as well:

    // Pipable sum
    FIT_STATIC_LAMBDA_FUNCTION(sum) = pipable([](auto x, auto y)
    {
        return x + y;
    });

As we will see, this can help make it cleaner when we are defining several lambdas, such as for overloading.

Overloading
-----------

Now, Fit provides two ways of doing overloading. The [`match`](match.md) adaptor will call a function based on C++ overload resolution, which tries to find the best match, like this:

    FIT_STATIC_LAMBDA_FUNCTION(print) = match(
        [](int x)
        {
            std::cout << "Integer: " << x << std::endl;
        },
        [](const std::string& x)
        {
            std::cout << "String: " << x << std::endl;
        }
    );

However, when trying to do overloading involving something more generic, it can lead to ambiguities. So the [`conditional`](conditional.md) adaptor will pick the first function that is callable. This allows ordering the functions based on which one is more important. Say we would like to write a `print` function that can print not only using `cout` but can also print the values in ranges. We could write something like this:


    FIT_STATIC_LAMBDA_FUNCTION(print) = conditional(
        [](const auto& x) -> decltype(std::cout << x, void())
        {
            std::cout << x << std::endl;
        },
        [](const auto& range)
        {
            for(const auto& x:range) std::cout << x << std::endl;
        }
    );

So the `-> decltype(std::cout << x, void())` will only make the function callable if `std::cout << x` is callable. Then the `void()` is used to return `void` from the function. We can constrain the second overload as well, but we will need some helper function in order to call `std::begin` using ADL lookup:

    namespace adl {

    using std::begin;

    template<class R>
    auto adl_begin(R&& r) -> FIT_RETURNS(begin(r));
    }

    FIT_STATIC_LAMBDA_FUNCTION(print) = conditional(
        [](const auto& x) -> decltype(std::cout << x, void())
        {
            std::cout << x << std::endl;
        },
        [](const auto& range) -> decltype(std::cout << *adl::adl_begin(range), void())
        {
            for(const auto& x:range) std::cout << x << std::endl;
        }
    );

Tuples
------

We could extend this to printing tuples as well. We will need to combine a couple of functions to make a `for_each_tuple`, which let us call a function for each element. First, the [`by`](by.md) adaptor will let us apply a function to each argument passed in, and the [`unpack`](unpack.md) adaptor will unpack the elements to a tuple and apply them to the argument:

    FIT_STATIC_LAMBDA_FUNCTION(for_each_tuple) = [](const auto& sequence, auto f) FIT_RETURNS
    (
        unpack(by(f))(sequence)
    );

So now we can add an overload for tuples:

    FIT_STATIC_LAMBDA_FUNCTION(print) = conditional(
        [](const auto& x) -> decltype(std::cout << x, void())
        {
            std::cout << x << std::endl;
        },
        [](const auto& range) -> decltype(std::cout << *adl::adl_begin(range), void())
        {
            for(const auto& x:range) std::cout << x << std::endl;
        },
        [](const auto& tuple) -> decltype(for_each_tuple(tuple, identity), void())
        {
            return for_each_tuple(tuple, [](const auto& x)
            {
                std::cout << x << std::endl;
            });
        }
    );

Since we can't use a lambda inside of `decltype` we just put [`identity`](identity.md) instead.

Recursive
---------

Even though we are using lambdas, we can easily make this recursive using the [`fix`](fix.md) adaptor. This implements a fix point combinator, which passes the function(ie itself) in as the first argument, so we could write this:

    FIT_STATIC_LAMBDA_FUNCTION(print) = fix(conditional(
        [](auto, const auto& x) -> decltype(std::cout << x, void())
        {
            std::cout << x << std::endl;
        },
        [](auto self, const auto& range) -> decltype(self(*adl::adl_begin(range)), void())
        {
            for(const auto& x:range) self(x);
        },
        [](auto self, const auto& tuple) -> decltype(for_each_tuple(tuple, self), void())
        {
            return for_each_tuple(tuple, self);
        }
    ));

Variadic
--------

We can also make this `print` function varidiac, so it prints every argument passed into it. We just rename our original `print` function to `simple_print`:

    FIT_STATIC_LAMBDA_FUNCTION(simple_print) = fix(conditional(
        [](auto, const auto& x) -> decltype(std::cout << x, void())
        {
            std::cout << x << std::endl;
        },
        [](auto self, const auto& range) -> decltype(self(*adl::adl_begin(range)), void())
        {
            for(const auto& x:range) self(x);
        },
        [](auto self, const auto& tuple) -> decltype(for_each_tuple(tuple, self), void())
        {
            return for_each_tuple(tuple, self);
        }
    ));

And then apply the [`by`](by.md) adaptor to `simple_print`:

    FIT_STATIC_LAMBDA_FUNCTION(print) = by(simple_print);

Requirements
============

This requires a C++11 compiler. There are no third-party dependencies. This has been tested on clang 3.4, gcc 4.6-4.9, and Visual Studio 2015 RC.
