# random-array-generator
This project was a product of a need for a vast amount of randomly-generated for testing my code in my algorithms class. 

## Project Summary
The _Dataset\<T, size, distT\>_ class is a wrapper around an array along with some convenience methods and support for random data generation of various types (random, sorted,
reverse sorted, nearly sorted, and few unique). The class is purely a wrapper in the truest sense as it has an overloaded `[]` operator (for array element access) as well
as an overloaded `T*` operator so that it can be passed as a parameter to a function that takes `T array[]`. When you pass this class to a parameter `T array[]`, it will return 
return a pointer to the internal array, losing access to the rest of the class members and methods.

Currently, this project only works with integers, but I have plans to update it to work with floating-point numbers.

## Usage Summary and Examples

| Code | Explanation |
| ---- | ----------- |
| `Dataset<int,100> arr` | assumes '_RANDOM_'; an array of 100 random integers. |
| `Dataset<int,100,RANDOM> arr` | an array of 100 random, integers. |
| `Dataset<int,100,SORTED> arr` | an array of 100 random, sorted integers. |
| `Dataset<int,100,REVERSE_SORTED> arr` | an array of 100 random, reverse-sorted integers. |
| `Dataset<int,100,NEARLY_SORTED> arr` | an array of 100 random, nearly sorted intgers |
| `Dataset<int,100,FEW_UNIQUE> arr` | an array of 100 random, few-unique integers |
> Nearly sorted arrays have {sqrt(sqrt(size))} unsorted elements. <br />
> Few-unique arrays {sqrt(size)} unique elements. <br />

## Compilation Instructions
This project is only a single header file, _Dataset.hpp_, which must be included in another project.

## License
This project is available under an MIT license. Do whatever you want with it — public or private.
