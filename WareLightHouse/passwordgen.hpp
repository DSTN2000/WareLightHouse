#ifndef PASSWORDGEN_HPP
#define PASSWORDGEN_HPP


#include <string>
#include <random>
#include <algorithm>

using namespace std;

struct PasswordGenerator{

    int randint(int min_value, int max_value){
        int delta = max_value-min_value;
        return round(min_value + (delta * ((double)rand() / RAND_MAX)));
    }

    string generatePassword()
    {
        string letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        string numbers = "0123456789";
        string symbols = "!#$%&()*+";

        string password;

        // ----# DETERMINES HOW MANY LETTERS, NUMBERS AND SYMBOLS THE GENERATED PASSWORD WILL HAVE #----
        int nr_letters = randint(8,10);
        int nr_symbols = randint(2,4);
        int nr_numbers = randint(2,4);

        // ----# ADDS RANDOM LETTERS, NUMBERS AND SYMBOLS IN ORDER #----
        for (int i = 0; i < nr_letters; i++) {
            password += letters[randint(0, letters.length()-1)];
        }

        for (int i = 0; i < nr_symbols; i++) {
            password += symbols[randint(0, symbols.length()-1)];
        }

        for (int i = 0; i < nr_numbers; ++i) {
            password += numbers[randint(0, numbers.length()-1)];
        }

        // ----# SHUFFLES THE PASSWORD #----
        shuffle(password.begin(), password.end(), mt19937(random_device()()));

        // ----# RETURNS THE SHUFFLED PASSWORD #----
        return password;
    }

};



#endif // PASSWORDGEN_HPP
