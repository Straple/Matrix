#define DEBUG_MODE // EASSERT verify

#include "matrix.h"
#include "time.h"

using namespace std;

int main() {

    // examples

    // eassert
    {
        // need to comment out

        // if defined DEBUG_MODE than EASSERT verify condition
        EASSERT(false, "hello world!");
        
        ERROR_REPORT(false, "always checks");
    }

    cout << "=====CONSTRUCTORS=====\n\n";
    // constructors
    {

        // container constructor
        // multitypes (long long vector) => int matrix
        matrix<int> M = vector<vector<long long>>{
               {1, 2, 3, 4, 5},
               {6, 7, 8, 9, 10},
               {11, 12, 13, 14, 15},
               {16, 17, 18, 19, 20},
               {21, 22, 23, 24, 25},
        };

        cout << "matrix M:\n" << M << "\n";
        //                    ^
        //                    |
        //--------------------+
        // operator << (ostream)

        matrix<int> M_other(vector<float>{1.12, 2.084, 3.5, 4.6, 5, 6.1, 7.29, 8}, 4, 2);

        cout << "other M:\n" << M_other << "\n";

        matrix<int> M_copy = M; // copy constructor

        matrix<int> M_copy_operator;
        M_copy_operator = M; // copy operator

        matrix<int> M_move_operator;
        M_move_operator = move(M); // move operator

        matrix<int> M_move = move(M_copy); // move constructor

        M_move.clear(); // clear

    } // destructors

#define print_with_name(x) #x << " = " << x << "\n"

    cout << "=====UTILS=====\n\n";
    // utils
    {
        matrix<string> S(3, 2, "hi");

        cout << "matrix S:\n" << S << "\n";

        cout << print_with_name(S.row_size()) <<
            print_with_name(S.col_size()) <<
            print_with_name(S.size()) <<
            print_with_name(S.empty()) << "\n";

        // S.shape() returns tuple(S.row_size(), S.col_size())


        S.reshape(1, 2, "f");
        cout << "reshape S:\n" << S << "\n";

    }

    cout << "=====AGGREGATE=====\n\n";
    // aggregate
    {
        matrix<int> M = vector<vector<long long>>{
               {1, 2, 3, 4, 5},
               {6, 7, 8, 9, 10},
               {11, 12, 13, 14, 15},
               {16, 17, 18, 19, 20},
               {21, 22, 23, 24, 25},
        };

        cout << "matrix M:\n" << M << "\n";

        cout << print_with_name(M.sum()) <<
            print_with_name(M.mean());


        /*
        как в векторе прописаны специальные typedef'ы
        using value_type = T;
	    using pointer = T*;
	    using const_pointer = const T*;
	    using reference = T&;
	    using const_reference = const T&;
	    using size_type = size_t;
	    using difference_type = ptrdiff_t;
        */
        decltype(M)::value_type prod = 1;
        M.aggregate([&prod](int value) {
            prod *= value;
        });

        cout << print_with_name(prod) << "\n";

        cout << "M + M aggregate:\n" << aggregate(M, M, [](int a, int b) {
            return a + b;
        }) << "\n";

        cout << "M + 3 aggregate:\n" << aggregate(M, [](int a) {
            return a + 3;
        }) << "\n";
         
        cout << "M + M:\n" << M + M << "\n";

        cout << "M^2:\n" << M * M << "\n";
    }

    cout << "=====TRANSFORM=====\n\n";
    // transform
    {
        matrix<int> M = vector<vector<long long>>{
              {1, 2, 3, 4, 5},
              {6, 7, 8, 9, 10},
              {11, 12, 13, 14, 15},
              {16, 17, 18, 19, 20},
              {21, 22, 23, 24, 25},
        };

        cout << "transpose M:\n" << M.transpose() << "\n";

        int n = -1;

        n = 0;
        cout << "gorizontal split. n = " << n << "\n\n" <<
            "first: " << M.gorizontal_split(n).first << "\n" << 
            "second: " << M.gorizontal_split(n).second << "\n";


        n = 2;
        cout << "vertical split. n = " << n << "\n\n" <<
            "first: " << M.vertical_split(n).first << "\n" <<
            "second: " << M.vertical_split(n).second << "\n";

        cout << "slice:\n" << M.slice(0, 1, 3, 4) << "\n";
    }

    // также есть итераторы: iterator, const_iterator, reverse_iterator
    // begin, end,
    // cbegin, cend,
    // rbegin, rend,
    // Они в примерах не нуждаются. Просто как у вектора. 
    // Но также можно узнать номер колонки и строки у итератора

    return 0;
}

#include <random>
mt19937_64 rnd(42);

int calc_time() {

    matrix<long long> M(1e4, 1e4);
    for (auto& it : M) {
        it = rnd();
    }

    Timer time;
    int n = 20;
    for (int i = 0; i < n; i++) {
        M.transpose();
    }

    double t = time.time();
    cout << "Summary: " << t << " Mean: " << t / n << "\n";


    return 0;
}

// Лог замеров времени выполнения

// матрица [1e3 x 1e3]
// operator *
// n = 30
// Summary: 57.7785 Mean : 1.92595
// Summary: 51.1253 Mean : 1.70417 transpose
// Summary: 25.8385 Mean: 0.861285 Ускорение кешем. Поменять местами две строчки кода.

// далее будут только матрицы [1e4 x 1e4]

// operator +, -, +=, -=, aggregate functions
// n = 1000
// Summary: 63.2574 Mean: 0.0632574 INT
// Summary: 135.752 Mean: 0.135752 LLONG
// ускорять уже некуда

// gorizontal_split
// n = 100
// Summary: 18.4729 Mean: 0.184729 INT
// Summary: 37.3909 Mean: 0.373909 LLONG
// Summary: 15.9091 Mean: 0.159091 INT std::memcpy
// Summary: 32.8869 Mean: 0.328869 LLONG std::memcpy
// memcpy дает приятный бонус к скорости

// делаем выводы, что long long в два раза длиньше. Разница в скорости тоже

// vertical_split
// n = 100
// Summary: 36.996 Mean: 0.36996 INT
// Summary: 78.6614 Mean: 0.786614 LLONG
// 
// Summary: 29.3457 Mean: 0.293457 INT std::memcpy
// Summary: 65.7117 Mean: 0.657117 LLONG std::memcpy

// slice
// n = 100
// всей матрицы: 
//    Summary: 19.2673 Mean: 0.192673 INT
//    Summary: 41.1203 Mean: 0.411203 LLONG
// некоторого куска:
//    Summary: 3.39617 Mean: 0.0339617 INT
//    Summary: 7.87771 Mean: 0.0787771 LLONG

// transpose
// n = 20
// Summary: 32.0216 Mean : 1.60108
// Summary: 28.3216 Mean: 1.41608 раскручивание в один цикл
