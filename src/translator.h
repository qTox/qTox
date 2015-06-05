#ifndef TRANSLATOR_H
#define TRANSLATOR_H

class QTranslator;

class Translator
{
public:
    /// Loads the translations according to the settings or locale
    static void translate();

private:
    static QTranslator* translator;
};

#endif // TRANSLATOR_H
