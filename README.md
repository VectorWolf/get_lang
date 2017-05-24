# get_lang
Language detection working with single words and text. Fast and customizable written in C++.

It trains on the enclosed wordbooks, is able to test itself spitting out success percentages and you can ask it your own words.
I also added support to train or test on custom user provided .txt files.

This program is fully modular, takes as much languages as you want (and your RAM is able to manage), 
has stacked patterns for 1 to ∞ letter constructions which are specific on the location etc.

It works like that:

    Letters
    |   Neuron Layers
    |   1   2   3   4    ...
    E - E   |   |   |
            EX  |   |
    X - X       EXA |
            XA      EXAM
    A - A       XAM
            AM      XAMP
    M - M       AMP
            MP      AMPL
    P - P       MPL
            PL      MPLE
    L - L       PLE  |
            LE   |   |
    E - E    |   |   |
        |    |   |   |
        sum+sum+sum+sum=chance
        
Its quite comparable to the "standard" n-gram algorithm, but it works per word and discriminates the patterns by their
individual position.
It converts the words as specified in the charset file, first char is the "end of word" sign, "-" by default.
It ignores chars specified in ignore.txt

The language files provided are from http://www.winedt.org/dict.html and I converted them to plain .txt files in utf_8.
They should work out of the box.

This project started as a simple port of my python language-recognition program, but it stores its ratings per pattern
differently. It needs less than 50% of RAM, about 25% of time and is a bit more successful than the python version.
