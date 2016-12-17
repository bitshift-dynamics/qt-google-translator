# qt-google-translator
Simple Qt integration of the Google Translator API.

This project provides the class GTranslator which can be used like the standard
Qt class QTranslator. Just install it once (in this case, not per language) and
then set the language you want your application translated to by calling
`GTranslator::setLanguage(const QString&)`.

You may also get a list of supported languages by calling
`GTranslator::requestLanguageList()`.

## Why this was made?
Simply because we can and its a nice idea ;)

But seriously, please dear companies - if you want your applications to be
localized, contract a proper translation service provider that is specialized
in translating software and not that 1cent per word far far east company.

If you do that, you can save your money and just use this little project. For
just 20 USD per 1 million characters (Google fees as of 17th December 2016)
it translates your application live in to over 100 languages. When you cache
your translated strings thats even cheaper than that crusty far far east
translator and the results are way better.

# How to build & use
Before you can build and use this project you'll have to provide a Google Cloud
Services API token. This is done by renaming the apitoken.tpl file to
apitoken.h and put your personal API token in the GoogleApiToken variable.

After setting your API token this project is built like any other simple Qt
qmake based project.

```bash
mkdir build
cd build
qmake ../qt-google-translator
make
./qt-google-translator
```
