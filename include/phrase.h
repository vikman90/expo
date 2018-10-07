#ifndef PHRASE_H
#define PHRASE_H

class Phrase {
public:
    static string name(mt19937 & random);
    static string password(mt19937 & random);
private:
    static const int NADJECTIVES;
    static const int NNOUNS;
    static const int NVERBS;
    static const char * adjectives[];
    static const char * nouns[];
    static const char * verbs[];
};

#endif
