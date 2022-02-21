# For autocompiling qm-files.

TRANSLATIONS = \
    translations/ar.ts \
    translations/be.ts \
    translations/ber.ts \
    translations/bg.ts \
    translations/cs.ts \
    translations/da.ts \
    translations/de.ts \
    translations/el.ts \
    translations/en.ts \
    translations/eo.ts \
    translations/es.ts \
    translations/et.ts \
    translations/fa.ts \
    translations/fi.ts \
    translations/fr.ts \
    translations/gl.ts \
    translations/he.ts \
    translations/hr.ts \
    translations/hu.ts \
    translations/is.ts \
    translations/it.ts \
    translations/ja.ts \
    translations/jbo.ts \
    translations/kn.ts \
    translations/ko.ts \
    translations/lt.ts \
    translations/mk.ts \
    translations/nl.ts \
    translations/no_nb.ts \
    translations/pl.ts \
    translations/pr.ts \
    translations/pt_BR.ts \
    translations/pt.ts \
    translations/ro.ts \
    translations/ru.ts \
    translations/si.ts \
    translations/sk.ts \
    translations/sl.ts \
    translations/sq.ts \
    translations/sr_Latn.ts \
    translations/sr.ts \
    translations/sv.ts \
    translations/sw.ts \
    translations/ta.ts \
    translations/tr.ts \
    translations/ug.ts \
    translations/uk.ts \
    translations/ur.ts \
    translations/zh_CN.ts \
    translations/zh_TW.ts

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
