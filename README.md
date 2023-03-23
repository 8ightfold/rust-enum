# rust-enum
This library is for those who miss using Rust enums in C++. It will give you the ability to create variants that can be matched. The only dependency is the [Boost Preprocessor](https://github.com/boostorg/preprocessor) library.

## Usage
To use ``rust-enum``, you must first create your enum. You can do this with the ``$enum`` macro, like so:
```cpp
$enum(name,
    (first),
    (second)(std::string, int),
    (third),
    (last)(std::vector<int>),
);
```
The first element in each declaration is the name, the second is the type (this will override the default enum type).

You can then use the ``$match`` macro to switch over the enum. The following is an example of this:
```cpp
$match(instance,
    (first) ( 
        std::cout << "first" << std::endl; 
    ),
    (second)(foo, bar) (
        std::cout << "second: " << foo << ", " << bar << std::endl;
    ),
    (_) (
        std::cout << "default." << std::endl;
    )
)
```
The first element in each case is the variant you wish to switch over, and the second is the name of the created instance. 
Passing ``_`` as the first argument without passing a name will set the ``default`` case.

``$match`` also works with virtuals. The syntax is the same, and can be used out of the box (no custom definitions required).

### Tips
While the ``$match`` macro is pretty, but isn't the most efficient. It has to generate paths for ``$enum`` and virtuals, even though only one is taken.
To speed up preprocessing, you can use ``$fmatch`` for ``$enum``s, and ``$vmatch`` for virtuals. These will take a lot less time to process.

You can find a working example on Godbolt [here](https://godbolt.org/z/vdqssGsGv).

## Notes
If you run into any issues or have an idea, let me know!
