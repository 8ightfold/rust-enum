# rust-enum
This library is for those who miss using Rust enums in C++. It will give you the ability to create variants that can be matched. The only dependency is the [Boost Preprocessor](https://github.com/boostorg/preprocessor) library.

## Usage
To use ``rust-enum``, you must first create your enum. You can do this with the ``$enum`` macro, like so:
```cpp
$enum(name,
    (first),
    (second)(std::string),
    (third),
    (last)(std::vector<int>),
);
```
The first element in each declaration is the name, the second is the type (this will override the default enum type).

You can then use the ``$match`` macro to switch over the enum. The following is an example of this:
```cpp
$match(instance,
    (first)(foo) (
        std::cout << "first: " << foo << std::endl;
    ),
    (_) (
        std::cout << "default." << std::endl;
    )
)
```
The first element in each case is the variant you wish to switch over, and the second is the name of the created instance. 
Passing ``_`` as the first argument without passing a name will set the ``default`` case.

You can find a working example on Godbolt [here](https://eightfold.godbolt.org/z/b5ejv3cG3).

## Notes
Tuple syntax is currently unsupported.
If you run into any issues or have an idea, let me know!
