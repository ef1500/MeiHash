<div align="center">

# MeiHash - Dual-Mixing Non-Cryptographic 128-Bit Hash

</br>

![Language](https://img.shields.io/badge/C%2FC%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Status](https://img.shields.io/badge/SMHasher-PASS-success?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

</div>

---

MeiHash is a non-cryptographic hash function, whose inspiration came from none other than Mei Aihara, the lead heroine of Citrus.

Meticulous, but easily taken aback when it comes to Yuzu. They make an interesting couple, but at the end of the day, like any other couple, they rely on one another to be strong. Similarly, MeiHash relies on two mixing paths to be strong, covering the weaknesses of the other. Two paths become one single hash value.
 
MeiHash achieves about 10.3 GB/s and passes all SMHasher tests. It uses a dual-mixing architecture to generate high-entropy values.

---

# Initial Idea 

<div align="center">
  <img src="https://github.com/user-attachments/assets/e783c176-d1fa-4eb0-ae66-d2b59664819e" height="600" alt="MeiHash Concept Note">
  <br>
  <em>The initial idea, scrawled down on some engineering paper during my abstract algebra class.</em>
</div>

<br>


The idea was as follows:

* Split the 128-bit data into two 64-bit halves, $M_i$ and $Y_i$
* One path XORs the data with the "secret" data ($M_S$, $Y_S$)
* One path XORs the data with the "seed" block  ($M_P$, $Y_P$)
* Both M and Y halves get multiplied together and get stored in $M_z$ and $M_k$.
*   $M_z = M_s * Y_s$ ,  $M_k = M_p * Y_p$
* XOR $M_z$ with $M_k$ to get the hash value

To my bewilderment, this approach worked better than expected. 

>[!NOTE]
>SMhasher uses a 32 bit seed, but meihash is designed to use a 64-bit key, so during testing the provided key from SMHasher had to be casted to `uint64_t` from `uint32_t`.

 # Design

 Meihash uses 4 lanes and 2 accumulators. Increasing the number of accumulators resulted in failures during the cyclic key tests on SMHasher.

 A Murmur-style avalanche technique is used to distribute the entropy across all the bits.

 # Test Results

 MeiHash passed all tests on SMHasher, and the results are included in this repository (see SMHasher.txt). All tests were run on Ubuntu (WSL2).

 | Metric | Result |
| :--- | :--- |
| Throughput | ~10.3 GB/s (Bulk) |
| Small Keys | ~26-30 cycles/hash |
| Bias | < 1% |
| SMHasher | PASS (All Tests) |

In addition, some following notable results from the tests:
* The worst bias in the avalanche test was with 96-bit keys, at `0.826667%`
* Zero collisions found in Sparse and Differential tests.

# Usage

### Requirements
MeiHash relies on the `__uint128_t` type, which restricts it to 64-bit systems (GCC, Clang, MSVC on x64).

To use it, simply download `meihash.c` and `meihash.h` and add `meihash.h` to your includes.

Example
```cpp
#include "meihash.h"
#include <stdio.h>

int main()
{
    // Input data
    const char* data = "I LOVE MY WIFE!";
    size_t len = 16;
    uint64_t seed = 0x4d414b415241ULL;
    
    // Output
    uint64_t hash[2];

    // Compute Hash
    meihash128(data, len, seed, hash);

    // Print
    printf("Hash: %016llx%016llx\n", 
           (unsigned long long)hash[1], 
           (unsigned long long)hash[0]);

    return 0;
}
```
