# For autocompiling qm-files.

TRANSLATIONS = translations/es.ts \
               translations/bg.ts \
               translations/de.ts \
               translations/fi.ts \
               translations/fr.ts \
               translations/hr.ts \
               translations/hu.ts \
               translations/it.ts \
               translations/nl.ts \
               translations/lt.ts \
               translations/mannol.ts \
               translations/pirate.ts \
               translations/pl.ts \
               translations/ru.ts \
               translations/sl.ts \
               translations/sv.ts \
               translations/uk.ts \
               translations/zh.ts \
               translations/pt.ts

#rules to generate ts
isEmpty(QMAKE_LUPDATE) {
    win32: QMAKE_LUPDATE = $$[QT_INSTALL_BINS]/lupdate.exe
    else:  QMAKE_LUPDATE = $$[QT_INSTALL_BINS]/lupdate
}

#limitation: only on ts can be generated
updatets.name = Creating or updating ts-files...
updatets.input = _PRO_FILE_
updatets.output = $$TRANSLATIONS
updatets.commands = $$QMAKE_LUPDATE ${QMAKE_FILE_IN}
updatets.CONFIG += no_link no_clean
QMAKE_EXTRA_COMPILERS += updatets

#rules for ts->qm
isEmpty(QMAKE_LRELEASE) {
    win32: QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease.exe
    else:  QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
updateqm.name = Compiling qm-files...
updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link  no_clean target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

# Release all the .ts files at once
updateallqm = $$QMAKE_LRELEASE -silent $$TRANSLATIONS
