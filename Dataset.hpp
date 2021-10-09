/*
  Author: Ethan Cox
  Date: 9/27/2021
  Compilation Instructions:
  Function: generates a unique array of X elements of numeric values between the values Y and Z of type T

  Usage examples:
  ===============
  'Dataset<int,20> array' is an array of 20 random integers, distributed RANDOM
  'Dataset<int,20,SORTED> array' is an array of 20 random, sorted integers, distributed RANDOMly
  'Dataset<int,20,REVERSE_SORTED> array' is an array of 20 random, sorted integers, distributed RANDOMly
  'Dataset<int,20,NEARLY_SORTED> array' is an array of 20, nearly-sorted integers
  'Dataset<int,20,FEW_UNIQUE> array' is an array of 20, few-unique integers
*/

//Header guard
#pragma once

//Native C++ Libraries
#include <iostream>         //Input/output
#include <iomanip>         //Formatting floats
#include <random>         //Random number generators
#include <algorithm>     //Sorting functions
#include <type_traits>  //Type-info for type-guarding
#include <new>         //Contains the std::bad_alloc exception

//Native C Libraries
#include <cstddef>     //Contains 'size_t'
#include <cmath>      //Contains 'sqrt()'


//Different types of distribution
enum Distribution { RANDOM, SORTED, REVERSE_SORTED, NEARLY_SORTED, FEW_UNIQUE };


//Dataset class contains a smart pointer to an array of random numeric values
template <typename T, size_t size, Distribution distT = RANDOM>  //Default distribution is 'RANDOM' (generic random dataset)
class Dataset
{
    //Guarding against non-numeric types and arrays of an invalid sizes
    static_assert(std::is_integral<T>::value, "Dataset class can only be of an integral type (int, unsigned int, short...etc)");
    static_assert(size > 0, "The size of the dataset must be a positive integer.");


    // DATA MEMBERS //
    private:
        T* array;                //Internal array, generated and freed automatically

    public:
        const size_t length;   //const!

    // FUNCTION MEMBERS //
    private:
        void genRandomData(T,T);        //Generate a new dataset, which is sorted if needed
        void genUniqueData(T,T);       //Generate a new dataset of few-unique data

    public:
        //Public special methods
        Dataset(T = 1000, T = 0);   //default maximum, minimum
        ~Dataset();

        //Public methods
        void genNewData(T = 1000, T = 0);    //Helper function: generates a new dataset of the appropriate type (RANDOM, SORTED, REVERSE_SORTED, NEARLY_SORTED, FEW_UNIQUE)
        void print() const;                 //Prints the array
        T* get();                          //Return a pointer to the internal array

        //Iterators
        T* begin() const;              //Beginning of the array
        T* end() const;               //End of the array

        //Operator overloads
        operator T*() const;       //Implicit conversion to pointer (for passing to T[])
        T& operator[](size_t);    //[] Overload for accessing class like 'arr[5] = ...'
};

/*
    +----------------------------+
    |   Dataset Implementation   |
    +----------------------------+
*/


// ********** SPECIAL MEMBER FUNCTIONS **********

//Constructor
template <typename T, size_t size, Distribution distT>
Dataset<T, size, distT>::Dataset(T max, T min): length(size)   //Initializer list for const data member
{
    //Initialize the array
    try
    {
        array = new T[size];
    }
    catch(const std::bad_alloc& e)
    {
        std::cerr << e.what() << '\n';
    }

    //Generate new data (random, sorted, reverse-sorted, nearly-sorted, or few-unique)
    genNewData(max, min);
}

//Deconstructor
template <typename T, size_t size, Distribution distT>
Dataset<T, size, distT>::~Dataset()
{
    //Automatically free internal array
    delete [] array;
}


// ********** PRIVATE METHODS ********** //

//Generate random data (for: RANDOM, SORTED, REVERSE_SORTED)
template <typename T, size_t size, Distribution distT>
void Dataset<T, size, distT>::genRandomData(T max, T min)
{
    //Create + seed Mersenne Twister random number generator
    std::random_device rd;
    std::mt19937 RNG(rd());

    //Apply distribution
    std::uniform_int_distribution<T> distribution(min, max);   // TODO: allow for float/double overload via 'if constexpr' conditional compilation

    //Fill the array with random values
    for(size_t i=0; i < size; i++)
    {
        //Generate a random value between the minimum and maximum
        array[i] = distribution(RNG);
    }

    //Sort?
    if constexpr (distT == SORTED or distT == NEARLY_SORTED)
    {
        //Sort in non-decreasing order (0 -> 1000, with repeats)
        std::sort(array, array + length, [](size_t i, size_t j) {return i < j;});  //lambda expression
    }

    //Reverse sort?
    if constexpr (distT == REVERSE_SORTED)
    {
        //Sort in non-increasing order (1000 -> 0, with repeats)
        std::sort(array, array + length, [](size_t i, size_t j) {return i > j;});  //lambda expression
    }

    //Nearly sorted? 
    if constexpr (distT == NEARLY_SORTED)
    {
        //Mess up the order :D! (just a bit)
        std::uniform_int_distribution<T> randomIndex(0, size-1);      //Random index generator 
        size_t amount = sqrt(sqrt(size));   

        for(size_t i=0; i < amount; i++)
        {
           //Swap two random element
           std::swap(array[randomIndex(RNG)], array[randomIndex(RNG)]);
        }
    }
}

//Generate few-unique data (for: FEW_UNIQUE)
template <typename T, size_t size, Distribution distT>
void Dataset<T, size, distT>::genUniqueData(T max, T min)
{
    //Create + seed Mersenne Twister random number generator
    std::random_device rd;
    std::mt19937 RNG(rd());

    //Apply distribution
    std::uniform_int_distribution<T> distribution(min, max);   // TODO: allow for float/double overload via 'if constexpr' conditional compilation

    /*
        FEW_UNIQUE Implementation:
        1. Fill the beginning of the array with {sqrt(size)} random elements
        2. Use the filled slots to randomly fill the rest of the dataset
        3. Remove the unique subset at the beginning of the array (the initial random elements) 
    */


    //The amount of unique elements is the square root of the size of the dataset
    size_t amount = sqrt(size);       //Still using 'size_t' because the square root of 18.422 quintillion is still larger than max size of an int 
    size_t i = 0;                    //The iterator 'i' delcare outside so it can span both loops

    //Apply random distributions
    std::uniform_int_distribution<T> randomSampleIndex(0, amount-1);   //Generate a random element of the sample list to draw from
    std::uniform_int_distribution<T> randomIndex(0, size-1);          //Generate a random index in the array

    //Populate the sample list with a few random values
    for(i; i < amount; i++)
    {
        //Generate a random value
        array[i] = distribution(RNG);
    }

    //Use the random sample to propagate the rest of the data
    for(i; i < size; i++)  
    {
        //Generate a random value from the sample set
        array[i] = array[randomSampleIndex(RNG)];
    }


    //Eliminate the unique subset at the beginning of the array (the sample list from the 1st for-loop)
    for(int j=0; j < amount; j++)
    {
        //Swap the current element from the sample list with a random element
        std::swap(array[j], array[randomIndex(RNG)]);
    }
}


// ********** PUBLIC METHODS **********

//Generate a new dataset
template <typename T, size_t size, Distribution distT>
void Dataset<T, size, distT>::genNewData(T max, T min)
{
    if constexpr (distT == FEW_UNIQUE)
        genUniqueData(max, min);
    else
        genRandomData(max, min);  //sorting is automatically taken care of here

}

//Return a pointer to the array (not really necessary because of implicit T* conversion)
template <typename T, size_t size, Distribution distT>
T* Dataset<T, size, distT>::get()
{
    return array;
}

//Print
template <typename T, size_t size, Distribution distT>
void Dataset<T, size, distT>::print() const
{
    //Print all the values of the array
    for(size_t i=0; i < size; i++)
    {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
}


// ********** ITERATORS ********** 

//Begin iterator
template <typename T, size_t size, Distribution distT>
T* Dataset<T, size, distT>::begin() const
{
    //Return the address of the first element in the array
    return &array[0];
}


//End iterator
template <typename T, size_t size, Distribution distT>
T* Dataset<T, size, distT>::end() const
{                            
    //Return the address of the last element in the array          
    return &array[0] + length;
}

// ********** OPERATOR OVERLOADING **********

//T* Conversion Overload (returns a pointer to the internal array of type T)
template <typename T, size_t size, Distribution distT>
Dataset<T, size, distT>::operator T*() const
{
    //The name of the array is a pointer to the first element
    return array;
}

//[] Overload
template <typename T, size_t size, Distribution distT>
T& Dataset<T, size, distT>::operator[](size_t index)
{
    return array[index];
}
