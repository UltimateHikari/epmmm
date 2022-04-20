#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cmath>
#include <chrono>
#include <immintrin.h>

float find_max(__m256 absed, float delta){
    alignas(32) float arr[8];
    _mm256_store_ps(arr, absed);
    for(int i = 0; i < 8; i++){
        delta = (arr[i] > delta ? arr[i] : delta);
    }
    return delta;
}
void check_delta_module(){
    // ненормальнейший костыль лол.. зато свое
    alignas(32) float arr[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, -1.0f, -2.0f};
    __m256 loaded = _mm256_load_ps(arr);

    const __m256 mask = _mm256_castsi256_ps(_mm256_set1_epi64x(0x7FFFFFFF7FFFFFFF));
    __m256 absed = _mm256_and_ps(loaded, mask);

    _mm256_store_ps(arr, absed);
    for(int i = 0; i < 8; i++){
        std::cout << arr[i] << " ";
    }
    float a = find_max(absed, 0.0f);
    std::cout << std::endl << a << std::endl;
}
void check_shift_with_blend(){
    alignas(32) float arr[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    alignas(32) float arr2[8] = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    alignas(32) float res[8];
    __m256 va = _mm256_load_ps(arr);
    __m256 vb = _mm256_load_ps(arr2);
    // postponed.
    // _mm256_store_ps(res, vc);
    // for(int i = 0; i < 8; i++){
    //     std::cout << res[i] << " ";
    // }
    // return;
}

int main(){
    check_delta_module();
    std::cout << std::endl;
    check_shift_with_blend();
    return EXIT_SUCCESS;
}