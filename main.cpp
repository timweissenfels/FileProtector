// Tim Weissenfels 2021

#include <iostream>
#include <fstream>
#include <future>
#include <chrono>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/multiprecision/miller_rabin.hpp>

using namespace boost::multiprecision;

typedef number<cpp_int_backend<2048, 2048, unsigned_magnitude, checked>> uint2048_t;
typedef number<cpp_int_backend<4096, 4096, unsigned_magnitude, checked>> uint4096_t;
typedef number<cpp_int_backend<8192, 8192, unsigned_magnitude, checked>> uint8192_t;

namespace std {
    template<> class numeric_limits<uint2048_t> : public std::numeric_limits<unsigned int> {
        public:
            static uint2048_t max() {
                uint2048_t ret = (std::numeric_limits<uint1024_t>::max)();
                return (ret*ret);
            };
    };
    
    template<> class numeric_limits<uint4096_t> : public std::numeric_limits<unsigned int> {
        public:
            static uint4096_t max() {
                uint4096_t ret = (std::numeric_limits<uint2048_t>::max)();
                return (ret*ret);
            };
    };

    template<> class numeric_limits<uint8192_t> : public std::numeric_limits<unsigned int> {
        public:
            static uint8192_t max() {
                uint8192_t ret = (std::numeric_limits<uint4096_t>::max)();
                return (ret*ret);
            };
    };
}

template <class T>
T generate_number(boost::mt19937 &random_generator, T max) {
    boost::random::uniform_int_distribution<T> dist(1, max);
    return dist(random_generator);
}

// Taken from https://stackoverflow.com/a/1900161/8964221
int getSeed() {
    std::ifstream rand("/dev/urandom");
    char tmp[sizeof(int)];
    rand.read(tmp,sizeof(int));
    rand.close();
    int* number = reinterpret_cast<int*>(tmp);
    return (*number);
}

//Return type defines key lenght (uint1024_t,uint2048_t uint4096_t,uint8192_t, etc.)
template <class T>
T get_key() {
    
    T max_num = (std::numeric_limits<T>::max)();
    boost::mt19937 random_generator((getSeed() + clock()));
    auto rand_num = generate_number(random_generator,max_num);
    boost::mt11213b second_generator(getSeed());

    auto prime_checker = []<typename G>(T num, G sec_gen) -> std::pair<bool,T> {
        if(miller_rabin_test(num, 45, sec_gen)) 
            return std::make_pair(true,num);
        return std::make_pair(false,num);
    };

    std::vector<std::future<std::pair<bool,T>>> primes;
    for(int i = 0; i < 100;i++)
        primes.emplace_back(std::async(prime_checker,generate_number(random_generator,max_num),second_generator));
    
    for(auto& element : primes) {
        auto values = element.get();
        if(values.first == true) 
            if(miller_rabin_test(values.second,10,second_generator))
                return values.second;
            else
                std::cout << "Fehler" << std::endl;
    }
            
    return rand_num;
}

int main() {
    std::cout << get_key<uint8192_t>() << std::endl;
    std::cout << get_key<uint8192_t>() << std::endl;
}
