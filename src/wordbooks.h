#ifndef WORDBOOKS_H_INCLUDED
#define WORDBOOKS_H_INCLUDED
#include <vector>
#include <string>
#include <map>
#include <set>
#include <limits>
#include <random>

using std::vector;
using std::map;
using std::string;
using std::set;

/**
 * @brief This class maintains language recognition data
 * During initialisation charsets and a conversion list are loaded from specified files.
 * Specified wordbooks are loaded from files, words are conversed and saved in Brain.wb.
 * Brain.mind is properly set up to learn on specified word ranges and pattern ranges.
 *
 * Init -> Train -> Test
 *
 */
class Brain {
public:
    /// Default constructor
    Brain(const unsigned minl, const unsigned maxl, const vector<string> langli, const unsigned plen = 0);

    /// Initialization routines
    void init_charsets(const string csfile = "../util/charset.txt");
    void init_ignore(const string csfile = "../util/ignore.txt");
    void init_conversion(const string cofile = "../util/conversion.txt");
    void import_wordbooks();

    /// Trains Brain.mind on random word specified in Brain.wb
    void train_random();
    void train_random(const unsigned lang_index);
    void train_random_bulk(const unsigned word_count);

    /// Tests Brain.mind on random word specified in Brain.wb
    unsigned test_random();
    unsigned test_random(const unsigned lang_index);
    void test_random_bulk(const unsigned word_count);
    double test_random_bulk_silent(const unsigned word_count);

    /// Inits small unchanged test pool of words
    void init_trial_wb(const unsigned word_count);
    /// Returns succes rates per language tested on full trial pool
    double test_trial() const;

    /// Tests Brain.mind on word specified by user
    void test_custom_word(string word);
    /// Couts propability rates per language of specified word-slice
    void test_custom_slice(string word, unsigned pos);

    /// Functions to set and evaluate optimal scale values
    void autoset_scale(unsigned word_count, unsigned pmin, unsigned pmax, double step);
    void test_scale(unsigned word_count, unsigned pmin, unsigned pmax, double step, double smin, double smax);
    void set_scale(unsigned pmin, unsigned pmax);

    void train_on_file(const string file, const unsigned lang_index);
    void test_on_file(const string file);

    const unsigned minlength; /// Minimum length of words
    const unsigned maxlength; /// Maximum length of words
    unsigned maxlength2; /// Actual maximum length (+1 end sign)
    const vector<string> langlist; /// List of language names
    const unsigned nlang; /// Count of languages
    vector<vector<vector<unsigned char>>> wb; /// Wordbook sorted by languages
    vector<vector<vector<unsigned char>>> trial_wb; /// Small wordbook for testing
    const vector<unsigned> init_rating; /// Default template for Brain.mind data
    unsigned max_pattern_len; /// The maximum relevant pattern length used
    double def_rating = 1.0 / nlang; /// Default rating per language if no val given
    vector<map<vector<unsigned char>, vector<unsigned>>> mind; /// All Ratings
    set<wchar_t> unidentified_chs {}; /// List of unidentified chars found by str_to_brwrd
    unsigned discard_count {0}; /// Count of discarded words by str_to_brwrd
    std::minstd_rand r_generator;

    unsigned polling_rate {10'000}; /// Rate at which progress of train or test is printed
    const unsigned MAX_VAL {std::numeric_limits<unsigned>::max()}; /// Maximum value of rate
    const unsigned char KILL_CHAR {255}; /// Char which indicates failed conversion
    vector<double> scale {}; /// Scale which amplifies ratings per pattern accordingly

private:
    /// Maps used by str_to_brwrd
    map<char, unsigned char> charset1;
    map<wchar_t, unsigned char> charset2;
    map<wchar_t, string> conversion;
    set<char> ignore1;
    set<wchar_t> ignore2;

    /// Converts String to brainword
    vector<unsigned char> str_to_brwrd(string word, bool check_len = true);
    /// Converts brainword to String
    string brwrd_to_str(vector<unsigned char> brwd) const;
    /// Halves rates (usually when MAX_VAL is reached)
    void shrink(unsigned pos, vector<unsigned char> slice);

    /// Trains Brain.mind on given word
    void train_single(vector<unsigned char> word, unsigned lang_index);
    /// Returns propability of languages on given word
    vector<double> test_single(vector<unsigned char> word) const;

    static string base_path;
};


#endif // WORDBOOKS_H_INCLUDED
