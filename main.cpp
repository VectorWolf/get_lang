#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include "wordbooks.h"
#include <random>

using namespace std;

int main() {

    setlocale(LC_CTYPE, "en_US.utf8");
    cout << "Welcome to Leif's neuron based language recognizer with dynamically "
            "stacked pattern size. Everything should be quite self explanatory. "
            "Use 0 for unlimitied values. The bigger \"maxpattern\" is, the more"
            " accurate and memory consuming the process will be.\n\n";
    unsigned minlength;
    unsigned maxlength;
    unsigned maxpattern;
    unsigned langlength;
    cout << "Minimum length of words: ";
    cin >> minlength;
    cout << "Maximum length of words: ";
    cin >> maxlength;
    cout << "Maximum pattern length: ";
    cin >> maxpattern;
    vector<string> namelist {"afr", "cze", "esp", "fre", "ger", "ita", "lat", "nla", "pol", "swe"};
    //vector<string> namelist {"fre", "ger", "ita"}; // smaller default langlist
    cout << "How many languages to learn?: ";
    cin >>langlength;
    if(langlength != 0) {
        namelist.resize(langlength);
        for(unsigned i = 0; i < langlength; i++) {
            string lang;
            cout << "Name of language file: ";
            cin >> lang;
            namelist[i] = lang;
        }
    }
    Brain Neurons(minlength, maxlength, namelist,maxpattern);

    while(true) {
        cout << "1 Train for x-times\n"
                "2 Test success rate for x-words\n"
                "3 Ask for language of a single word\n"
                "4 Get chances for word-slice\n"
                "5 Pattern scaling, etc.\n"
                "6 Exit\n";
        char decide;
        cout << "Decision: ";
        cin >> decide;
        cout << "\n";
        switch(decide) {
        case '1': {
            cout << "How many random words to train on? : ";
            unsigned int train_words;
            cin >> train_words;
            Neurons.train_random_bulk(train_words);
            cout << "\n";
            }break;

        case '2': {
            cout << "How many random words to test on? : ";
            unsigned test_words;
            cin >> test_words;
            Neurons.test_random_bulk(test_words);
            cout << "\n";
            }break;

        case '3': {
            cout << "Which word to test on? : ";
            string custom_word;
            cin >> custom_word;
            cout << "\n";
            Neurons.test_custom_word(custom_word);
            cout << "\n";
            }break;

        case '4': {
            cout << "Which slice to test on? : ";
            string custom_slice;
            cin >> custom_slice;
            cout << "At which position? : ";
            unsigned pos;
            cin >> pos;
            cout << "\n";
            Neurons.test_custom_slice(custom_slice, pos);
            cout << "\n";
            }break;

        case '5': {
                char decide {'0'};
                while(decide != '6') {
                    cout << "1 Evaluate and set optimal scaling\n"
                            "2 Print success per scaling steps\n"
                            "3 Manually set scaling values\n"
                            "4 Train on custom file\n"
                            "5 Test on custom file\n"
                            "6 Return\n";
                    cout << "Decision: ";
                    cin >> decide;
                    cout << "\n";
                    switch(decide) {
                    case '1': {
                        cout << "Size of testing wordbook : ";
                        unsigned wordcount;
                        cin >> wordcount;
                        cout << "Minimum pattern : ";
                        unsigned minpattern;
                        cin >> minpattern;
                        cout << "Maximum pattern : ";
                        unsigned maxpattern;
                        cin >> maxpattern;
                        cout << "Step size : ";
                        double step;
                        cin >> step;
                        Neurons.autoset_scale(wordcount, minpattern, maxpattern, step);
                        cout << "\n";
                        }break;

                    case '2': {
                        cout << "Size of testing wordbook : ";
                        unsigned wordcount;
                        cin >> wordcount;
                        cout << "Minimum pattern : ";
                        unsigned minpattern;
                        cin >> minpattern;
                        cout << "Maximum pattern : ";
                        unsigned maxpattern;
                        cin >> maxpattern;
                        cout << "Step size : ";
                        double step;
                        cin >> step;
                        cout << "Starting scale size : ";
                        double minscale;
                        cin >> minscale;
                        cout << "Ending scale size : ";
                        double maxscale;
                        cin >> maxscale;
                        Neurons.test_scale(wordcount, minpattern, maxpattern, step, minscale, maxscale);
                        cout << "\n";
                        }break;

                    case '3': {
                        cout << "Minimum pattern : ";
                        unsigned minpattern;
                        cin >> minpattern;
                        cout << "Maximum pattern : ";
                        unsigned maxpattern;
                        cin >> maxpattern;
                        for(unsigned i = minpattern; i <= maxpattern; i++) {
                            cout << "Value for " << i << "-size patterns (old: " << Neurons.scale[i] << " ) : ";
                            cin >> Neurons.scale[i];
                        }
                        cout << "\n";
                        }break;

                    case '4': {
                        cout << "File? (without .txt) : ";
                        string file;
                        cin >> file;
                        cout << "Possible lang index\n";
                        for(unsigned i = 0; i < Neurons.nlang; i++) cout << i << " " << Neurons.langlist[i] << " ";
                        cout << "\nLang index? : ";
                        unsigned lang_index;
                        cin >>lang_index;
                        cout << "\n";
                        Neurons.train_on_file(file, lang_index);
                        cout << "\n";
                        }break;

                    case '5': {
                        cout << "File? (without .txt) : ";
                        string file;
                        cin >> file;
                        cout << "\n";
                        Neurons.test_on_file(file);
                        cout << "\n";
                        }break;

                    case '6': {
                        }break;

                    default: {
                        cout << "\nPlease repeat!\n\n";
                        }break;
                    }
                }
            }break;

        case '6': {
            exit(0);
            }break;

        default: {
            cout << "\nPlease repeat!\n\n";
            }break;
        }
    }
    return 0;
}
