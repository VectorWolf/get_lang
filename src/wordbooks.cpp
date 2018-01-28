#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <limits>
#include <chrono>
#include <random>
#include "split.h"
#include "wordbooks.h"

using namespace std;

string Brain::base_path = "../wordbooks/";

/**
 * @brief Fully inits Brain class object
 *
 * Charsets and conversion list are initialized, wordbooks are imported.
 * Mind is set up to work with provided maximum word and pattern sizes.
 * Random Number generator gets initialized with current timestamp.
 *
 * @param minl, maxl, plen are passed
 * @param langli gets passed and its size is used to define init_rating and nlang
 *
 */
Brain::Brain(const unsigned minl, const unsigned maxl, const vector<string> langli, const unsigned plen)

: minlength {minl}, maxlength {maxl}, langlist{langli}, nlang{static_cast<unsigned>(langlist.size())},
init_rating(langlist.size() + 1, 0), max_pattern_len{plen}
{
    init_charsets();
    init_ignore();
    init_conversion();
    import_wordbooks();

    mind.resize(maxlength2);

    if(max_pattern_len == 0) scale.resize(maxlength2, 1.0);
    else scale.resize(max_pattern_len, 1.0);

    const unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    r_generator.seed(seed);

    cout << "\nInitialization done!\n" << endl;
}

/**
 * @brief Charset1 and charset2 are initialized from specified file
 *
 */
void Brain::init_charsets(const string csfile) {
    cout << "Starting import of charset\n";
    ifstream source(csfile);
    if (!source) {
        cerr << csfile <<" can't be opened!\n";
    }
    else {
        string line;
        for(unsigned char i = 0; getline(source, line); i++) {
            vector<string> characters = split(line, ':');
            for(string ch : characters) {
                if (ch.size() == 1) charset1[ch[0]] = i;
                else {
                    wchar_t wch {};
                    mbtowc(&wch, &ch[0], 6);
                    charset2[wch] = i;
                }
                cout << ch << " <-> " << static_cast<int>(i) << "\n";
            }
        }
    }
    cout <<"\n";
}

/**
 * @brief Brain.ignore is initialized from specified file
 *
 */
void Brain::init_ignore(const string csfile) {
    cout << "Starting import of ignored chars\n";
    ifstream source(csfile);
    if (!source) {
        cerr << csfile <<" can't be opened!\n";
    }
    else {
        string line;
        for(unsigned char i = 0; getline(source, line); i++) {
            if(line.size() == 1) {
                ignore1.insert(line[0]);
                cout << line[0] << " is ignored!\n";
            }
            else if(line.size() > 1) {
                wchar_t wch {};
                mbtowc(&wch, &line[0], 6);
                ignore2.insert(wch);
                cout << line << " is ignored! (wide char)\n";
            }
        }
    }
    cout <<"\n";
}

/**
 * @brief Conversion is initialized from specified file
 *
 */
void Brain::init_conversion(const string cofile) {
    cout << "Starting import of conversion-list\n";
    ifstream source(cofile);
    if (!source) {
        cerr << cofile << " can't be opened!\n";
    }
    else {
        string orig;
        string repl;
        while(getline(source, orig, ':') && getline(source, repl, '\n')) {
            wchar_t wch {};
            mbtowc(&wch, &orig[0], 6);
            conversion[wch] = repl;
            cout << orig << " <-> " << repl << "\n";
        }
    }
    cout <<"\n";
}

/**
 * @brief Imports wordbooks to object
 *
 * Wordbooks specified in Brain.langlist are converted word by word and
 * written into Brain.wb sorted by language.
 * The amount of discarded words gets counted and printed.
 * All unknow chars which occured during import are printed.
 * Also checks if randomizer is able to handle the amount of words per wb.
 *
 */
void Brain::import_wordbooks() {
    cout << "Starting import of wordbooks\n";
    wb.resize(langlist.size());
    unsigned maxwlen{};
    for(unsigned i = 0; i < langlist.size(); i++) {
        ifstream source(base_path + langlist[i] + ".txt");
        if (!source) {
            cerr << langlist[i] << "can't be opened!\n";
        }
        else {
            string word;
            while(getline(source, word)) {
                vector<unsigned char> c_word;
                c_word = str_to_brwrd(word);
                if(c_word[0] != KILL_CHAR) {
                    wb[i].push_back(c_word); // Word starting with '255' are discarded
                    if(c_word.size() > maxwlen) maxwlen = static_cast<unsigned>(c_word.size());
                }
                else discard_count++;
            }
            cout << "Language " << langlist[i] << " has got " << wb[i].size() << " Words\n";
            if(wb[i].size() > minstd_rand::max()) {
                cerr << "ERROR: Maximum random value is smaller than wordbook size!";
                exit(-1);
            }
        }
    }
    maxlength2 = maxwlen;
    if (discard_count != 0) cout <<"\n" << discard_count << " words were discarded because they included one of the following letters or had an invalid length:\n";
    ofstream source("unidentified.txt");
    for(wchar_t wch : unidentified_chs) {
        char ch[7] {};
        wctomb(ch,wch);
        cout << ch << " ";
        if (source) source << ch << "\n";
    }
}

/**
 * @brief Converts word to brainword which are used in Brain
 *
 * Single chars get converted as specified in charset1, charset2 and converse.
 * If unknown chars occur or the length of the word does not match the
 * minimum or maximum length the function returns the KILL_CHAR at pos 0.
 * Chars specified in Brain.ignore are simply ignored.
 *
 * @param word String which gets converted
 * @param check_len specifies if words with bad length are treated or not
 * @return fully converted brainword or KILL_CHAR
 *
 */
vector<unsigned char> Brain::str_to_brwrd(string word, bool check_len) {
    vector<unsigned char> brwrd;
    for(size_t i = 0; i < word.size(); i++) {
        if (charset1.find(word[i]) != charset1.end()) {
            brwrd.push_back(charset1[word[i]]);
        }
        else if(ignore1.find(word[i]) != ignore1.end()) {
            continue;
        }
        else {
            wchar_t wch{};
            int mbsize = mbtowc(&wch, &word[i], 6);
            if (mbsize <= 0) {
                cerr << word << " at pos " << i << " has improper multi-byte characters!";
                exit(-1);
            }
            if (charset2.find(wch) != charset2.end()) {
                brwrd.push_back(charset2[wch]);
                i += mbsize - 1;
            }
            else if (ignore2.find(wch) != ignore2.end()) {
                continue;
            }
            else if (conversion.find(wch) != conversion.end()) {
                word.replace(i, mbsize, conversion[wch]);
                i--;
            }
            else {
                unidentified_chs.insert(wch);
                //cerr << "The byte of \"" << word << "\" at pos " << i << " isn't recognized\n";
                return vector<unsigned char> {KILL_CHAR};
            }
        }
    }
    if(check_len) {
        if(brwrd.size() < minlength || brwrd.size() == 0) return vector<unsigned char> {KILL_CHAR};
        else if(maxlength > 0 && brwrd.size() > maxlength) return vector<unsigned char> {KILL_CHAR};
    }
    brwrd.push_back(0);
    return brwrd;
}

/**
 * @brief Trains Brain.mind on a single specified word
 *
 * @param word The word to train on
 * @param lang_index The index of the language of the word
 *
 */
void Brain::train_single(vector<unsigned char> word, unsigned lang_index) {
    unsigned plen{};
    if(max_pattern_len == 0 || word.size() < max_pattern_len) plen = word.size();
    else plen = max_pattern_len;

    for(unsigned i = 1; i <= plen; i++) {

        for(unsigned j = 0; j <= word.size() - i; j++) {
            vector<unsigned char> slice(&word[j],&word[j+i]);
            if(mind[j].find(slice) == mind[j].end()) mind[j][slice] = init_rating;
            if(mind[j][slice][0] == MAX_VAL) shrink(j,slice);
            mind[j][slice][0] += 1;
            mind[j][slice][lang_index + 1] += 1;
        }
    }
    return;
}

/**
 * @brief Shrinks specified data of Brain.mind in half
 *
 * Primarily used to prevent an overflow of unsigned int.
 *
 * @param pos Position of the slice
 * @param slice Slice of a word
 *
 */
void Brain::shrink(unsigned pos, vector<unsigned char> slice) {
    unsigned sum {0};
    for(unsigned i = 1; i <= nlang; i++) {
        mind[pos][slice][i] /= 2;
        sum += mind[pos][slice][i];
    }
    mind[pos][slice][0] = sum;
}

/**
 * @brief Trains Brain.mind on a random word of Brain.wb
 */
void Brain::train_random() {
    unsigned lang_index = r_generator() % wb.size();
    unsigned word_index = r_generator() % wb[lang_index].size();
    train_single(wb[lang_index][word_index], lang_index);
    return;
}

/**
 * @brief Trains Brain.mind on a random word of given language index
 * @param lang_index Given language index
 */
void Brain::train_random(const unsigned lang_index) {
    unsigned word_index = r_generator() % wb[lang_index].size();
    train_single(wb[lang_index][word_index], lang_index);
    return;
}

/**
 * @brief Trains Brain.mind on an amount of random words of Brain.wb
 * @param word_count Amount of random words
 */
void Brain::train_random_bulk(const unsigned word_count) {
    cout << "Starting Training on " << word_count << " words.\n";
    for(unsigned i = 0; i < word_count; i++) {
        train_random();
        if(i % polling_rate == 0) {
            float progress = i * 100.f / word_count;
            cout << progress << "% done" << endl;
        }
    }
    cout << "100% done" << endl;
    return;
}

/**
 * @brief Tests a single word
 *
 * A single specified word is tested against data provided from Brain.mind .
 * It returns propabilities per language for the word.
 *
 * @param word Specified word to test
 * @return A vector containing propabilities for each language
 *
 */
vector<double> Brain::test_single(vector<unsigned char> word) const {
    unsigned plen{};
    if(max_pattern_len == 0 || word.size() < max_pattern_len) plen = word.size();
    else plen = max_pattern_len;

    vector<double> ratings(nlang,0);
    for(unsigned i = 1; i <= plen; i++) {
        vector<double> rating_per_pattern(nlang, 0);
        for(unsigned j = 0; j <= word.size() - i; j++) {
            vector<unsigned char> slice(&word[j],&word[j+i]);
            if(mind[j].find(slice) != mind[j].end()) {
                unsigned sum = mind[j].at(slice)[0];
                for(unsigned k = 1; k <= nlang; k++) {
                    rating_per_pattern[k-1] += static_cast<double>(mind[j].at(slice)[k]) / sum;
                }
            }
            else for(unsigned k = 0; k < nlang; k++) {
                rating_per_pattern[k] += def_rating;
            }
        }
        for(unsigned k = 0; k < nlang; k++) {
            rating_per_pattern[k] /= word.size() - i + 1;
            ratings[k] += scale[i-1] * (rating_per_pattern[k] - 0.5) + 0.5;
        }

    }
    for(unsigned i = 0; i < nlang; i++) ratings[i] /= plen;
    return ratings;
}

/**
 * @brief Tests a random word
 *
 * A random word in one of the wbs is tested.
 * Success gets determined.
 *
 * @return If success, the actual language index of the word
 *         If failure, the actual language index of the word + amount of languages
 *
 */
unsigned Brain::test_random() {
    const unsigned lang_index = r_generator() % wb.size();
    const unsigned word_index = r_generator() % wb[lang_index].size();
    vector<double> ratings = test_single(wb[lang_index][word_index]);
    unsigned choice{0};
    for(unsigned i = 0; i<nlang; i++) {
        if(ratings[i] > ratings[choice]) choice = i;
    }
    if(choice == lang_index) return lang_index;
    else return lang_index + nlang;
}

/**
 * @brief Tests a random word of a language
 *
 * A random word in one the specified wb is tested.
 * Success gets determined.
 *
 * @return If success, the actual language index of the word
 *         If failure, the actual language index of the word + amount of languages
 *
 */
unsigned Brain::test_random(const unsigned lang_index) {
    const unsigned word_index = r_generator() % wb[lang_index].size();
    vector<double> ratings = test_single(wb[lang_index][word_index]);
    unsigned choice{0};
    for(unsigned i = 0; i<nlang; i++) {
        if(ratings[i] < ratings[choice]) choice = i;
    }
    if(choice == lang_index) return lang_index;
    else return lang_index + nlang;
}

/**
 * @brief Tests an amount of words and prints success rates per language
 * @param word_count Amount of words
 *
 */
void Brain::test_random_bulk(const unsigned word_count) {
    cout << "Starting Testing on " << word_count << " words.\n";
    vector<unsigned> amounts(nlang, 0);
    vector<unsigned> hits(nlang, 0);
    for(unsigned i = 0; i < word_count; i++) {
        unsigned result = test_random();
        if (result >= nlang) result -= nlang;
        else hits[result]++;
        amounts[result]++;
        if(i % polling_rate == 0) {
            float progress = i * 100.f / word_count;
            cout << progress << "% done" << endl;
        }
    }
    cout << "100% done\n" << endl;
    unsigned overall_a {};
    unsigned overall_h {};
    for(unsigned i = 0; i < nlang ; i++) {
        cout << langlist[i] << " success: " << 100.0 * hits[i] / amounts[i]<< "%\n";
        overall_a += amounts[i];
        overall_h += hits[i];
    }
    cout << "\nOverall success: " << 100.0 * overall_h / overall_a << "%\n";
    return;
}

/**
 * @brief Tests an amount of words
 * @param word_count Amount of words
 * @return Success rates per language
 *
 */
double Brain::test_random_bulk_silent(const unsigned word_count) {
    vector<unsigned> amounts(nlang, 0);
    vector<unsigned> hits(nlang, 0);
    for(unsigned i = 0; i < word_count; i++) {
        unsigned result = test_random();
        if (result >= nlang) result -= nlang;
        else hits[result]++;
        amounts[result]++;
    }
    unsigned overall_a {};
    unsigned overall_h {};
    for(unsigned i = 0; i < nlang ; i++) {
        overall_a += amounts[i];
        overall_h += hits[i];
    }
    return 100.0 * overall_h / overall_a;
}

/**
 * @brief Initialises small wb for testing purposes
 * @param word_count The overall amount of words in trial_wb
 *
 */
void Brain::init_trial_wb(const unsigned word_count) {
    trial_wb.clear();
    trial_wb.resize(langlist.size());
    for(unsigned i=0; i < nlang; i++) {
        for(unsigned j=0; j < word_count / nlang; j++) {
            unsigned word_index = r_generator() % wb[i].size();
            trial_wb[i].push_back(wb[i][word_index]);
        }
    }
}

/**
 * @brief Tests all words in trial_wb
 * @return Success rates per language
 *
 */
double Brain::test_trial() const{
    unsigned amount {0};
    unsigned hits {0};
    for(unsigned i = 0; i < nlang; i++) {
        for(const vector<unsigned char> word : trial_wb[i]) {
            vector<double> ratings = test_single(word);
            unsigned choice{0};
            for(unsigned j = 0; j<nlang; j++) {
                if(ratings[j] > ratings[choice]) choice = j;
            }
            if(choice == i) hits++;
            amount++;
        }
    }
    return 100.0 * hits / amount;
}

/**
 * @brief Tests specified word, prints propabilities per language and its choice
 * @param word Specified word to test
 *
 */
void Brain::test_custom_word(string word) {
    vector<unsigned char> c_word = str_to_brwrd(word, false);
    if (c_word.size() > maxlength2) {
        vector<unsigned char> temp(&c_word[0],&c_word[maxlength2]);
        c_word = temp;
    }
    if (c_word[0] == KILL_CHAR) cout << "Invalid String given!\n";
    else {
        vector<double> rating = test_single(c_word);
        unsigned choice = 0;
        for(unsigned i = 0; i < nlang; i++) {
            if(rating[i] > rating[choice]) choice = i;
            cout << langlist[i] << " success: " << rating[i] * 100 << "%\n";
        }
        cout << "\n I choose " << langlist[choice] << " !\n";
    }
    return;
}

/**
 * @brief Prints propabilities of word slice at a position
 * @param word Slice to test
 * @param pos Position of the slice
 *
 */
void Brain::test_custom_slice(string word, unsigned pos) {
    if(pos != 0) pos--;
    else {
    cout << "Range is from 1 to maxlength\n";
    return;
    }
    vector<unsigned char> slice = str_to_brwrd(word, false);
    if(slice[0] == KILL_CHAR) cout << "Invalid String given!\n";
    else {
        slice.pop_back();
        //string sl_word = brwrd_to_str(slice);
        //cout << string(pos, '_') << sl_word << "is being tested\n";
        if(mind[pos].find(slice) != mind[pos].end()) {
            unsigned sum = mind[pos].at(slice)[0];
            for(unsigned i = 1; i <= nlang; i++) {
                double chance = static_cast<double>(mind[pos].at(slice)[i]) / sum;
                cout << langlist[i - 1] << " chance: " << chance * 100 << "%\n";
            }
        }
        else {
            cout << "slice not available!\n";
        }
    }
    return;
}

/**
 * @brief Tests and sets optimal scale values per pattern size
 *
 * The test basis is trial_wb to ensure minimal randomness during testing.
 * It increases scaling of specified pattern ranges individually and checks
 * if the succes rates rise. It stops upon worse or same success rates and
 * changes back to previous step.
 * It prints all values during testing and the final scale values at the end.
 *
 * @param word_count Amount of words used per testing (more means less randomness)
 * @param pmin Minimum pattern length
 * @param pmax Maximum pattern length
 * @param step Increasement per step
 *
 */
void Brain::autoset_scale(unsigned word_count, unsigned pmin,unsigned pmax, double step) {
    init_trial_wb(word_count);
    for(unsigned i=pmin - 1; i < pmax; i++) {
        double success_new = test_trial();
        double success_old;
        cout << success_new << " at " << scale[i] << "  " << i+1 << "-patterns\n";
        do {
            success_old = success_new;
            scale[i] += step;
            success_new = test_trial();
            cout << success_new << " at " << scale[i] << "  " << i+1 << "-patterns\n";
        } while(success_new > success_old);
        scale[i] -= step;
        cout << "\n";
    }
    cout << "\n";
    for(unsigned i = pmin - 1; i < pmax; i++) {
        cout << i+1 << "-patterns   " << scale[i] << "-scaled\n";
    }
}

/**
 * @brief Tests scale values per pattern size
 *
 * The test basis is trial_wb to ensure minimal randomness during testing.
 * It increases scaling of specified pattern ranges individually and tests.
 * Then it changes back to the old values.
 * It prints all values during testing.
 *
 * @param word_count Amount of words used per testing (more means less randomness)
 * @param pmin Minimum pattern length
 * @param pmax Maximum pattern length
 * @param step Increasement per step
 * @param smin Starting value of scale
 * @param smax End value of scale
 *
 */
void Brain::test_scale(unsigned word_count, unsigned pmin, unsigned pmax, double step, double smin, double smax) {
    init_trial_wb(word_count);
    for(unsigned i=pmin - 1; i < pmax; i++) {
        double old_sc = scale[i];
        double success;
        for(double j = smin; j <= smax; j += step) {
            scale[i] = j;
            success = test_trial();
            cout << success << " at " << scale[i] << "  " << i+1 << "-patterns\n";
        }
        scale[i] = old_sc;
        cout << "\n";
    }
}

/**
 * @brief Trains on all words in specified file
 *
 * Reads line by line, word by word and trains only on valid words within min- and maxlength.
 *
 * @param file Specified file without .txt, which hast to be utf-8
 * @param lang_index The index of the correspondig language of the file
 *
 */
void Brain::train_on_file(const string file, const unsigned lang_index) {
    cout << "Starting training on " << file << "\n";
    ifstream source("../" + file + ".txt");
    if (!source) {
        cerr << file <<" can't be opened!\n";
    }
    else {
        vector<double> ratings(nlang, 0);
        string line;
        unsigned counter {0};
        for(unsigned i = 1; getline(source, line); i++) {
            vector<string> word_list = split(line, ' ');
            for(string word : word_list) {
                vector<unsigned char> brwrd = str_to_brwrd(word);
                if(brwrd[0] == KILL_CHAR) continue;
                train_single(brwrd, lang_index);
                counter++;
                if(counter % polling_rate == 0) cout << i << " lines and " << counter << " words trained\n";
            }
        }
        cout << counter << " words trained\n";
    }
}

/**
 * @brief Tests on all words in specified file
 *
 * Reads line by line, word by word and tests on all words. Words which are too long ar cut down to maxlength.
 * Prints rates per language and choice.
 *
 * @param file Specified file without .txt, which hast to be utf-8
 *
 */
void Brain::test_on_file(const string file) {
    cout << "Starting test on " << file << "\n";
    ifstream source("../" + file + ".txt");
    if (!source) {
        cerr << file <<" can't be opened!\n";
    }
    else {
        vector<double> ratings(nlang, 0);
        string line;
        unsigned counter {0};
        for(unsigned i = 1; getline(source, line); i++) {
            vector<string> word_list = split(line, ' ');
            for(string word : word_list) {
                vector<unsigned char> brwrd = str_to_brwrd(word, false);
                if(brwrd[0] == KILL_CHAR) continue;
                if (brwrd.size() > maxlength2) {
                    vector<unsigned char> temp(&brwrd[0],&brwrd[maxlength2]);
                    brwrd = temp;
                }
                vector<double> rates = test_single(brwrd);
                for(unsigned j = 0; j < nlang; j++) ratings[j] += rates[j];
                counter++;
                if(counter % polling_rate == 0) cout << i << " lines and " << counter << " words tested\n";
            }
        }
        cout << counter << " words tested\n\n";
        unsigned choice {0};
        for(unsigned i = 0; i < nlang; i++) {
            if(ratings[i] > ratings[choice]) choice = i;
            cout << langlist[i] << " success: " << ratings[i] / counter * 100 << "%\n";
        }
        cout << "\n I choose " << langlist[choice] << " !\n";
    }
}
