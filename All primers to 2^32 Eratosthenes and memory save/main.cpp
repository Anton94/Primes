#include <iostream>
#include <cmath>

using std::cout;
using std::cerr;
using std::pair;

// NOTE:
// I keep the buckets left to right, BUT in the bucket I start from the least significant bit to most sign. bit
// And I will make the assumption that one byte = 8 bits.
class BitVector
{
private:
    unsigned char * buff;
    unsigned int cntBuckets; // Number of buckets(variables)
    unsigned int cntBitsLastBucket; // Number of bits in the last bucket, because the bits in the vector might be not multiple of 8.
public:
    // Range[0, size] CLOSED range
    BitVector(unsigned int size)
    {
        // Corner case: size === 1...111 in binary
        if (size == (unsigned int) - 1)
        {
            cntBuckets = (size >> 3) + 1; // Closed range, I need to add one more bit, so adds new bucket for it.
            cntBitsLastBucket = 1;
        }
        else
        {
            cntBuckets = (size + 1) >> 3;
            cntBitsLastBucket = (size + 1) & 7; // (size + 1) % 8
            if (!cntBitsLastBucket) // If the residual of (size + 1) % 8 == 0 then the last bucket is full
                cntBitsLastBucket = 8;
            else // Place for the last few bits
                ++cntBuckets;
        }

        // In one unsigned char variable I will keep 1 byte of bits.
        buff = new (std::nothrow) unsigned char[cntBuckets];
        if (!buff)
            cerr << "Not enough memory for the buffer!\n";
        else
            resetBits();
    }

    ~BitVector()
    {
        delete[] buff;
    }
public:
    // Returns positive number if the bit at the given @position is 1 and zero if the bit is 0.
    unsigned char checkBit(unsigned int index) const
    {
        // I need index / 8 to find the bucket, where the number is stored
        // and in each bucket(unsigned char) there are 8 of them, so i need index % 8 to find the exact position in the bucket
        // index / 8 === index >> 3
        // index % 8 === index & 7 // 7 in binary = 000...0111
        // Now I have the index of the bit in the bucket and I can make a mask with only one '1' at this index position
        // For example, if index % 8 = 3 -> then the mask will be 00..01000 which is (1 << (index % 8))
        // And I can extract that value when I apply logical AND on the bucket with the that mask.
        return buff[index >> 3] & (1 << (index & 7));
    }

    // Sets the bit, in the vector, at the given index position.
    void setBit(unsigned int index)
    {
        buff[index >> 3] |= 1 << (index & 7);
    }

    // Deletes the allocated memory.
    void free()
    {
        delete [] buff;
        buff = NULL;
        cntBuckets = 0;
        cntBitsLastBucket = 0;
    }
    // Sets all bits to 0.
    void resetBits()
    {
        for (unsigned int i = 0; i < cntBuckets; ++i)
                buff[i] = 0;
    }

    void print() const
    {
        unsigned int bound = (cntBuckets == 0 ? 1 : cntBuckets) - 1;
        for (unsigned int i = 0; i < bound; ++i) // For each bucket
        {
            // print binary
            for (unsigned int j = 0; j < 8; ++j)
            {
                cout << (buff[i] & (1 << j) ? "1" : "0");
            }
        }
        // Last bucket might be with just few bits.
        for (unsigned int j = 0; j < cntBitsLastBucket; ++j)
        {
            cout << (buff[bound] & (1 << j) ? "1" : "0");
        }
        cout << "\n";
    }

    // Returns the number of bits which are 0 in the vector. Works faster when the 1's are >> than the 0's
    // Works for vectors with sizes multiple of 8!
    unsigned int countOfZeroBits() const
    {
        unsigned int res = 0;
        unsigned int bound = (cntBuckets == 0 ? 1 : cntBuckets) - 1;
        for (unsigned int i = 0; i < bound; ++i) // For each bucket
        {
            unsigned char tmp = ~buff[i]; // Invert the bits, so now I count the 0's in the original number
            while(tmp)
            {
                tmp &= (tmp - 1);
                ++res;
            }
        }

        for (unsigned int j = 0; j < cntBitsLastBucket; ++j)
        {
            if(!(buff[bound] & (1 << j)))
                ++res;
        }

        return res;
    }
private:
    BitVector(const BitVector& o);
    BitVector& operator=(const BitVector& o);
};

// Returns the number of primes. If there is not enough memory for the algorithm - returns -1
// Uses the passed as argument bit vector(if there is one), otherwise allocates one.
unsigned int numberOfPrimes(unsigned int size, BitVector * vec = NULL)
{
    if (size < 2)
        return 0;
    bool buff = vec;
    if (!buff)
    {
        vec = new (std::nothrow) BitVector(size);
        if (!vec)
            return -1;
    }

    for (unsigned int i = size & (unsigned int) - 2; i >= 4; i-=2) // unsigned int -2 === 111.....1110 , so I can set the last bit to 0, and the size to be EVEN number
        vec->setBit(i);
    vec->setBit(0);
    vec->setBit(1);

    unsigned int bound = ceil(sqrt(size));
    unsigned int current = 3, count = 1;
    while (current <= bound)
    {
        // Set all multiplies of current
        unsigned int twoCurrent = current << 1; // current * 2
        unsigned int powTwoCurrent = current * current; // current is at most sqrt(N), so currentPowTwo is at most N
        // While j is less or equal to size and there is no overflow.
        // Corner case: current == bound -> powToCurrent overflows and it's 0 -> problem
        // So, another check j > current
        for (unsigned int j = powTwoCurrent; j <= size && j >= powTwoCurrent && j > current; j += twoCurrent)
            vec->setBit(j);

        ++count;
        // Find the next prime
        while (vec->checkBit(++current) && current <= bound)
            ;
    }

    do
    {
        if(!vec->checkBit(current))
            ++count;
    } while (current++ < size);

//    cout << "Number of primes is: " << vec->countOfZeroBits() << "\n"; // On my machine-slower

    if (!buff)
    {
        delete vec;
        vec = NULL;
    }

    return count;
}


// Returns the number of primes from 0 to 2^32. (Or error -1)
unsigned int numberOfPrimesTo2ToThePow32()
{
    unsigned int countOfPrimes = 0;
    // Determine the primes in the first sqrt(2^32) block.
    unsigned int sqrtOf2ToThePow32 = 1 << 16;
    BitVector firstBlock(sqrtOf2ToThePow32); // Range [0, 2^32]
    unsigned int numberOfPrimesInFirstBlock = numberOfPrimes(sqrtOf2ToThePow32, &firstBlock); // Now 0 bits in the vector are primes.(index of the bit is prime number).
    countOfPrimes += numberOfPrimesInFirstBlock;
    /*
         I will need each prime from the first block to eliminate the numbers in the tempBlock
         (eliminate each number in them block, which has some prime, from the first block, as multiplier.
         I will keep array with @numberOfPrimesInFirstBlock elements and use the values as counters.
         The number of elements is ~2^16/log(2^16), which is ~ 2^12 elements.
         For example: if unsigned int is 4 bytes, the memory use will be 2^12 * 2^2 bytes = 2^14 bytes
                      (and another 4 bytes for the original value of the prime number, so I can iterate with it in the next blocks)NOTE I can use unsigned short int for the prime value, it's enough
                      and for the vector to store 2^16 bits is 2^13 bytes
                      which is approximately 8 times as much memory, but less operations(maybe). (also at one point of time I will keep them both in memory, which is not so good..)

        If I use this kind of algorithm, I will iterate only over the primes, of first block...

        Some of the operations will be very good for intrinsics (SIMD operations).
    */

    pair<unsigned int, unsigned int> * primes = new (std::nothrow) pair<unsigned int, unsigned int>[numberOfPrimesInFirstBlock];
    if(!primes)
    {
        cerr << "There is not enough memory for the algorithm...";
        return -1;
    }

    // Now I need to insert the values of the primes into @primes array.
    for (unsigned int i = 0, idx = 2; i < numberOfPrimesInFirstBlock; ++i)
    {
        /* primes[i] = idx, but after that I need to add this primer(@idx) to itself until it get out of the range [0, sqrt(2^32)]
           So the value in primes[i] will be from the next block. (so @idx will be multiple of the value primes[i])
           [idx, sqrt(2^32)] range is sqrtN - idx.
           Divide the range by the prime number(@idx) and that will be the number of additions I need to make to get to the end of the range.
           And add one more addition to get out of the range.
           Also adds one more @idx because we start from idx, not from 0. (idx << 1) == idx * 2
        */
        primes[i].first = ((sqrtOf2ToThePow32 - idx) / idx) * idx + (idx << 1); // it's not the same as (sqrt - idx) + idx!
        primes[i].second = idx;

        // Find the next prime.
        while (firstBlock.checkBit(++idx))
                ;
    }

    // I don't need any more the first block.
    firstBlock.free();

    // The block size between two blocks is sqrt(2^32), BUT I need the elements in range [sqrt(2^32) + 1, 2*sqrt(2^32)]. (exclude the left border)
    BitVector tempBlock(sqrtOf2ToThePow32 - 1);
    unsigned int leftBound = sqrtOf2ToThePow32 + 1;
    unsigned int rightBound = sqrtOf2ToThePow32 << 1; // 2 * sqrtN
    for (unsigned int k = 1; k < sqrtOf2ToThePow32; ++k)
    {
        // Iterate through each prime counter from @primes and eliminate the numbers in the tempBlock
        for (unsigned int i = 0; i < numberOfPrimesInFirstBlock; ++i)
        {
            while(primes[i].first <= rightBound && primes[i].first >= sqrtOf2ToThePow32) // Second condition is because of overflows
            {
                tempBlock.setBit(primes[i].first - leftBound);
                primes[i].first += primes[i].second;
            }
        }

        countOfPrimes += tempBlock.countOfZeroBits();
        tempBlock.resetBits();
        leftBound = rightBound + 1;
        rightBound += sqrtOf2ToThePow32;
        if (rightBound == 0) // Border case - right border = 2^32
            rightBound = (unsigned int) - 1;
    }

    // In the last cycle I'am missing 2^32 to make it non-prime, because in primes[i].first the value is at most 2^32 - 1, also in the rightBound.
    --countOfPrimes;

    delete [] primes;

    return countOfPrimes;
}


// Returns the number of primes from 0 to 2^32. (Or error -1)
unsigned int numberOfPrimesTo2ToThePow32Second()
{
    unsigned int countOfPrimes = 0;
    // Determine the primes in the first sqrt(2^32) block.
    unsigned int sqrtOf2ToThePow32 = 1 << 16;
    BitVector firstBlock(sqrtOf2ToThePow32); // Range [0, 2^32]
    unsigned int numberOfPrimesInFirstBlock = numberOfPrimes(sqrtOf2ToThePow32, &firstBlock); // Now 0 bits in the vector are primes.(index of the bit is prime number).
    countOfPrimes += numberOfPrimesInFirstBlock;

    // The block size between two blocks is sqrt(2^32), BUT I need the elements in range [sqrt(2^32) + 1, 2*sqrt(2^32)]. (exclude the left border)
    BitVector tempBlock(sqrtOf2ToThePow32 - 1);
    unsigned int leftBound = sqrtOf2ToThePow32 + 1, currentMultiplier;
    for (unsigned int k = 1; k < sqrtOf2ToThePow32; ++k)
    {
        // Iterate through each prime counter from @primes and eliminate the numbers in the tempBlock
        for (unsigned int i = 2; i <= sqrtOf2ToThePow32; ++i)
        {
            if (!firstBlock.checkBit(i)) // If the @i number in the first block is prime.
            {
                currentMultiplier = ((leftBound - 1 - i) / i) * i + (i << 1); // leftBound - 1 because leftBound is the start of the next range, and it is valid value
                currentMultiplier -= leftBound; // I have the starting value in the new range
                // but to put it in the range [0, sqrtOf2ToThePow32)

                while(currentMultiplier < sqrtOf2ToThePow32)
                {
                    tempBlock.setBit(currentMultiplier);
                    currentMultiplier += i;
                }
            }
        }

        countOfPrimes += tempBlock.countOfZeroBits();
        tempBlock.resetBits();
        leftBound += sqrtOf2ToThePow32;
    }

    return countOfPrimes;
}



void testPrimesRangeCount(unsigned int size)
{
    cout << "Number of primes in range [0, " << size << "] is: " << numberOfPrimes(size) << "\n";
}



int main()
{
//    testPrimesRangeCount(0);
//    testPrimesRangeCount(1 << 0);
//    testPrimesRangeCount(1 << 1);
//    testPrimesRangeCount(1 << 2);
//    testPrimesRangeCount(1 << 3);
//    testPrimesRangeCount(1 << 4);
//    testPrimesRangeCount(1 << 8);
//    testPrimesRangeCount(1 << 16);
//    testPrimesRangeCount((unsigned int)-1); // 2 ^ 32 - 1 in unsigned int variable



  //  testPrimesRangeCount((unsigned int)-1); // A little slower than the other two, and more than 1000 times more memory
  //  cout << numberOfPrimesTo2ToThePow32() << "\n"; // On my machine a little faster than numberOfPrimesTo2ToThePow32Second
    cout << numberOfPrimesTo2ToThePow32Second() << "\n"; // On my machine a little slower, but a little less memory (than numberOfPrimesTo2ToThePow32)
    return 0;
}











//
//
//
//
//
//
//void testBitVectorSetAllBits(unsigned int size)
//{
//    cout << "Test - set all bits to 1 with vector size " << size << "\n";
//    BitVector v(size);
//    for (unsigned int i = 0; i < size; ++i)
//        v.setBit(i);
//    bool result = true;
//    for (unsigned int i = 0; i < size; ++i)
//    {
//        if (!v.checkBit(i))
//        {
//            result = false;
//            break;
//        }
//    }
//    cout << "\ttest... " << (result ? "passed" : "failed") << "!\n";
//}
//
//void testBitVectorSetEvenBits(unsigned int size)
//{
//    cout << "Test - set even bits to 1 with vector size " << size << "\n";
//    BitVector v(size);
//    for (unsigned int i = 0; i < size; i += 2)
//        v.setBit(i);
//    bool result = true;
//    for (unsigned int i = 0; i < size; i += 2)
//    {
//        if (!v.checkBit(i))
//        {
//            result = false;
//            break;
//        }
//    }
//    for (unsigned int i = 1; i < size; i += 2)
//    {
//        if (v.checkBit(i))
//        {
//            result = false;
//            break;
//        }
//    }
//    cout << "\ttest... " << (result ? "passed" : "failed") << "!\n";
//}
//
//void testBitVectorSetBits()
//{
//    unsigned int size = sizeof(unsigned int) * 8;
//    for (unsigned int i = 0; i < size; ++i)
//    {
//        testBitVectorSetAllBits(1 << i);
//        testBitVectorSetEvenBits(1 << i);
//    }
//}
//
//void testBitVectorDiffSizes(unsigned int size)
//{
//    cout << "Test - creating bit vector with size " << size << "\n";
//    BitVector v(size);
//    v.print();
//}
