/*
  Author: Ethan Cox
  Date: 10/9/2021
  Version: C++17
  Compilation Instructions: Header file. N/A.
  Function: generates a unique array of X elements of numeric values between the values Y and Z of type T

  Usage examples:
  ===============
  'Dataset<int,20> array' is an array of 20 random integers
  'Dataset<int,20, DT::SORTED> array' is an array of 20 random, sorted integers
  'Dataset<int,20, DT::REVERSE_SORTED> array' is an array of 20 random, sorted integers
  'Dataset<int,20, DT::NEARLY_SORTED> array' is an array of 20, nearly-sorted integers
  'Dataset<int,20, DT::FEW_UNIQUE> array' is an array of 20, random, few-unique integers
*/

//Header guard
#pragma once

//Native C++ Libraries
#include <iostream>          //Input/output
#include <iomanip>          //Formatting floats
#include <random>          //Random number generators
#include <algorithm>      //Sorting functions
#include <type_traits>   //Type-info for type-guarding
#include <stdexcept>    //Contains 'std::invalid_argument'

//Native C Libraries
#include <cstddef>     //Contains 'size_t'
#include <cmath>      //Contains 'sqrt()'


//Different types of datasets (as an enum class for type safety)
enum class DT { RANDOM, SORTED, REVERSE_SORTED, NEARLY_SORTED, FEW_UNIQUE };


//Dataset class contains a smart pointer to an array of random numeric values
template <typename T, size_t size, DT dataT = DT::RANDOM>  //Default distribution is 'RANDOM' (generic random dataset)
class Dataset
{
    //Guarding against non-numeric types
    static_assert(std::is_integral<T>::value, "Dataset class can only be of an integral type (int, unsigned int, short...etc)");
    static_assert(not std::is_same<char, T>::value and not std::is_same<wchar_t, T>::value, "Dataset objects must be integral, not character");


    // DATA MEMBERS //
    private:
        T dataset[size];                //Internal array

    public:
        const size_t length;   //const!

    // FUNCTION MEMBERS //
    private:
        void genRandomData(const T, const T);        //Generate a new dataset, which is sorted if needed
        void genUniqueData(const T, const T);       //Generate a new dataset of few-unique data

    public:
        //Public special methods
        constexpr Dataset(const T = 0, const T = 1000);   //default minimum, maximum

        //Public methods
        void genNewData(const T = 0, const T = 1000);    //Helper function: generates a new dataset of the appropriate type (RANDOM, SORTED, REVERSE_SORTED, NEARLY_SORTED, FEW_UNIQUE)
        void print() const;                             //Prints the array
        constexpr T* get() const noexcept;             //Return a pointer to the internal array

        //Iterators
        constexpr T* begin() const noexcept;              //Beginning of the array
        constexpr T* end() const noexcept;               //End of the array

        //Operator overloads
        operator T*();             //Implicit conversion to pointer (for passing to T[])
        T& operator[](size_t);    //[] Overload for accessing class like 'arr[5] = ...'
};

/*
    +----------------------------+
    |   Dataset Implementation   |
    +----------------------------+
*/


// ********** SPECIAL MEMBER FUNCTIONS **********

//Constructor
template <typename T, size_t size, DT dataT>
constexpr Dataset<T, size, dataT>::Dataset(const T min, const T max): length(size)   //Initializer list for const data member
{
    if (max < min)
        throw std::invalid_argument("invalid range; maximum cannnot be less than the minimum.");

    //Generate new data (random, sorted, reverse-sorted, nearly-sorted, or few-unique)
    genNewData(max, min);
}

// ********** PRIVATE METHODS ********** //

//Generate random data (for: RANDOM, SORTED, REVERSE_SORTED)
template <typename T, size_t size, DT dataT>
void Dataset<T, size, dataT>::genRandomData(const T min, const T max)
{
    if (max < min)
        throw std::invalid_argument("invalid range; maximum cannnot be less than the minimum.");

    //Create + seed Mersenne Twister random number generator
    std::random_device rd;
    std::mt19937 RNG(rd());

    //Apply distribution
    std::uniform_int_distribution<T> distribution(min, max);   // TODO: allow for float/double overload via 'if constexpr' conditional compilation

    //Fill the array with random values
    for(size_t i=0; i < size; i++)
    {
        //Generate a random value between the minimum and maximum
        dataset[i] = distribution(RNG);
    }

    //Sort?
    if constexpr (dataT == DT::SORTED or dataT == DT::NEARLY_SORTED)
    {
        //Sort in non-decreasing order (0 -> 1000, with repeats)
        std::sort(dataset, dataset + length, [](size_t i, size_t j) {return i < j;});  //lambda expression
    }

    //Reverse sort?
    if constexpr (dataT == DT::REVERSE_SORTED)
    {
        //Sort in non-increasing order (1000 -> 0, with repeats)
        std::sort(dataset, dataset + length, [](size_t i, size_t j) {return i > j;});  //lambda expression
    }

    //Nearly sorted? 
    if constexpr (dataT == DT::NEARLY_SORTED)
    {
        //Mess up the order :D! (just a bit)
        std::uniform_int_distribution<T> randomIndex(0, size-1);      //Random index generator 
        size_t amount = sqrt(sqrt(size));   

        for(size_t i=0; i < amount; i++)
        {
           //Swap two random element
           std::swap(dataset[randomIndex(RNG)], dataset[randomIndex(RNG)]);
        }
    }
}

//Generate few-unique data (for: FEW_UNIQUE)
template <typename T, size_t size, DT dataT>
void Dataset<T, size, dataT>::genUniqueData(const T min, const T max)
{
    if (max < min)
        throw std::invalid_argument("invalid range; maximum cannnot be less than the minimum.");

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
        dataset[i] = distribution(RNG);
    }

    //Use the random sample to propagate the rest of the data
    for(i; i < size; i++)  
    {
        //Generate a random value from the sample set
        dataset[i] = dataset[randomSampleIndex(RNG)];
    }


    //Eliminate the unique subset at the beginning of the array (the sample list from the 1st for-loop)
    for(int j=0; j < amount; j++)
    {
        //Swap the current element from the sample list with a random element
        std::swap(dataset[j], dataset[randomIndex(RNG)]);
    }
}


// ********** PUBLIC METHODS **********

//Generate a new dataset
template <typename T, size_t size, DT dataT>
void Dataset<T, size, dataT>::genNewData(const T min, const T max)
{
    if constexpr (dataT == DT::FEW_UNIQUE)
        genUniqueData(max, min);
    else
        genRandomData(max, min);  //sorting is automatically taken care of here

}

//Return a pointer to the array (not really necessary because of implicit T* conversion)
template <typename T, size_t size, DT dataT>
constexpr T* Dataset<T, size, dataT>::get() const noexcept
{
    //Return a pointer to the internal array
    return &dataset[0];
}

//Print
template <typename T, size_t size, DT dataT>
void Dataset<T, size, dataT>::print() const
{
    //Print all the values of the array
    for(size_t i=0; i < size; i++)
    {
        std::cout << dataset[i] << " ";
    }
    std::cout << std::endl;
}


// ********** ITERATORS ********** 

//Begin iterator
template <typename T, size_t size, DT dataT>
constexpr T* Dataset<T, size, dataT>::begin() const noexcept
{
    //Return the address of the first element in the array
    return &dataset[0];
}


//End iterator
template <typename T, size_t size, DT dataT>
constexpr T* Dataset<T, size, dataT>::end() const noexcept
{                            
    //Return the address of the last element in the array          
    return &dataset[0] + length;
}

// ********** OPERATOR OVERLOADING **********

//T* Conversion Overload (returns a pointer to the internal array of type T)
template <typename T, size_t size, DT dataT>
Dataset<T, size, dataT>::operator T*()
{
    //The name of the array is a pointer to the first element
    return dataset;
}

//[] Overload
template <typename T, size_t size, DT dataT>
T& Dataset<T, size, dataT>::operator[](const size_t index)
{
    return dataset[index];
}
