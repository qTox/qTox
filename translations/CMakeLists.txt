#   Copyright © 2020 by The qTox Project Contributors
#
#   This file is part of qTox, a Qt-based graphical interface for Tox.
#   qTox is libre software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   qTox is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with qTox.  If not, see <http://www.gnu.org/licenses/>

# Edit this list if translations are added
qt5_add_translation(translations_FILES
  ar.ts
  ber.ts
  be.ts
  bg.ts
  cs.ts
  da.ts
  de.ts
  el.ts
  en.ts
  eo.ts
  es.ts
  et.ts
  fa.ts
  fi.ts
  fr.ts
  gl.ts
  he.ts
  hr.ts
  hu.ts
  is.ts
  it.ts
  ja.ts
  jbo.ts
  kn.ts
  ko.ts
  lt.ts
  mk.ts
  nl.ts
  no_nb.ts
  pl.ts
  pr.ts
  pt_BR.ts
  pt.ts
  ro.ts
  ru.ts
  si.ts
  sk.ts
  sl.ts
  sq.ts
  sr_Latn.ts
  sr.ts
  sv.ts
  sw.ts
  ta.ts
  tr.ts
  ug.ts
  uk.ts
  ur.ts
  zh_CN.ts
  zh_TW.ts
)

# Generate a resource file which contains all translation files
function(generate_translation_resource)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc.in"
    "<!DOCTYPE RCC>
    <RCC version=\"1.0\">
      <qresource prefix=\"/translations\">
    ")
    foreach(qm ${translations_FILES})
      get_filename_component(qm_name ${qm} NAME)
      file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc.in"
        "    <file alias=\"${qm_name}\">${qm}</file>\n")
    endforeach(qm)
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc.in"
    "  </qresource>
    </RCC>
    ")
endfunction()

generate_translation_resource()
configure_file(${CMAKE_CURRENT_BINARY_DIR}/translations.qrc.in ./translations.qrc COPYONLY)

add_library(translations_library STATIC ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)

# An explicit dependency is needed or AUTORCC will run before the translation files are created
set_target_properties(translations_library PROPERTIES AUTOGEN_TARGET_DEPENDS "${translations_FILES}")
