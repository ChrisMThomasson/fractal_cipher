/* Reverse Iteration Fractal Cipher (RIFC)
by: Chris M. Thomasson
_______________________________________________________________ */


#include <cstdio>
#include <cstddef>
#include <climits>
#include <cfloat>
#include <complex>
#include <string>
#include <algorithm>

typedef std::complex<double> ct_complex;

#define CT_PI2 6.283185307179586476925286766559
#define CT_UINT_BITS (sizeof(unsigned int) * CHAR_BIT)

static std::string g_ct_rifc_symbols("0123456789ABCDEF");

#define CT_PREC "0.15"

unsigned int ct_max_symbol_index(std::string const& symbols)
{
    unsigned int max = 0;

    std::size_t n = symbols.length();

    for (std::size_t i = 0; i < n; ++i)
    {
        unsigned int symbol_index = g_ct_rifc_symbols.find(symbols[i]);
        max = std::max(max, symbol_index);
    }

    return max + 1;
}


bool
ct_compare(double a, double b, double e)
{
    double d = std::abs(b - a);
    return d < e;
}

bool ct_compare_complex(ct_complex a, ct_complex b, double e)
{
    bool r = ct_compare(a.real(), b.real(), e);
    bool i = ct_compare(a.imag(), b.imag(), e);
    return r && i;
}


ct_complex
ct_root(
    ct_complex const& z,
    unsigned int r,
    unsigned int b
) {
    double angle_base = std::arg(z) / b;
    double angle_step = CT_PI2 / b;

    double length = std::abs(z);

    double angle = angle_base + angle_step * r;
    double radius = std::pow(length, 1.0 / b);

    ct_complex root(
        std::cos(angle) * radius,
        std::sin(angle) * radius
    );

    return root;
}

unsigned int
ct_find(
    ct_complex const& z0,
    ct_complex const& z1,
    unsigned int b,
    double e
) {
    for (unsigned int i = 0; i < b; ++i)
    {
        ct_complex root = ct_root(z0, i, b);
        if (ct_compare_complex(z1, root, e)) return i;
    }

    return 0;
}





ct_complex
ct_rifc_store(
    ct_complex const& z0,
    ct_complex const& c,
    std::string const& symbols,
    unsigned int base
) {
    ct_complex z = z0;

    std::size_t n = symbols.length();

    for (std::size_t i = 0; i < n; ++i)
    {
        unsigned int symbol_index = g_ct_rifc_symbols.find(symbols[i]);

        z = ct_root(z - c, symbol_index, base);

        std::printf("ct_rifc_store:%c:%u:(%" CT_PREC "lf, %" CT_PREC "lf)\n",
            symbols[i], symbol_index, z.real(), z.imag());
    }

    return z;
}


std::string
ct_rifc_load(
    ct_complex const& z0, // Origin 
    ct_complex const& c,  // Secret Key... ;^)
    ct_complex const& z1, // Return Value From (ct_rifc_store)
    unsigned int base,    // Base,
    unsigned int n        // Number of symbols to load
) {
    ct_complex z = z1;
    double e = .0001;
    std::string symbols;

    for (unsigned int i = 0; i < n; ++i)
    {
        ct_complex sz = std::pow(z, base) + c;

        unsigned int symbol_index = 0;

        // If base is two, the the sign of the real 
        // part of sz can be used to decode.
        if (base == 2)
        {
            symbol_index = (z.real() < 0) ? 1 : 0;
        }

        else
        {
            symbol_index = ct_find(sz - c, z, base, e);
        }

        char symbol = g_ct_rifc_symbols[symbol_index];

        symbols = symbols + symbol;

        std::printf("ct_rifc_load:%c:%u:(%" CT_PREC "lf, %" CT_PREC "lf)\n",
            symbol, symbol_index, z.real(), z.imag());

        z = sz;
    }

    std::reverse(symbols.begin(), symbols.end());

    return symbols;
}


int main()
{

    {
        ct_complex z(0, 0);
        ct_complex c(-.75, .09);
        std::size_t complex_size = sizeof(ct_complex) * CHAR_BIT;

        std::printf("complex_size:%lu\n\n", (unsigned long)complex_size);

        std::printf("symbols:%s\n", g_ct_rifc_symbols.c_str());
        std::printf("_____________________________________\n");

        std::string stored_symbols(
            "11011111011111110111111111111111"  // 32 bits
            "11101111101110111111111111111111"  // 32 bits
            "11111101111111111111110111111111"  // 32 bits
            "11111111111101111111111111111111"  // 32 bits
            "11111111111111111111111111111111"  // 32 bits
            "11111111111111011111111111111111"  // 32 bits
            "11111111111111111111101111111111"  // 32 bits
            "11111111111111111111111111111100"  // 32 bits
        );

        std::size_t stored_symbols_size = stored_symbols.length();
        std::printf("stored_symbols.length():%lu\n",
            (unsigned long)stored_symbols_size);



        unsigned int base = ct_max_symbol_index(stored_symbols);

        std::printf("stored_symbols:%s\n", stored_symbols.c_str());
        ct_complex sz = ct_rifc_store(z, c, stored_symbols, base);
        std::printf("_____________________________________\n");

        std::string loaded_symbols = ct_rifc_load(z, c, sz, base, stored_symbols_size);
        std::printf("loaded_symbols:%s\n", loaded_symbols.c_str());
        std::printf("_____________________________________\n");

        if (stored_symbols != loaded_symbols)
        {
            std::printf("\n\nDATA CORRUPTED!!!\n\n");
        }

        else
        {
            std::printf("\n\nDATA IS COHERENT!!!\n\n");
        }
    }

    std::printf("\n\nProgram Complete!\n");
    std::printf("Hit <ENTER> to exit...\n");
    std::fflush(stdout);
    std::getchar();

    return 0;
}
