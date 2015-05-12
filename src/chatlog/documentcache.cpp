/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "documentcache.h"
#include "customtextdocument.h"

DocumentCache::~DocumentCache()
{
    while (!documents.isEmpty())
        delete documents.pop();
}

QTextDocument* DocumentCache::pop()
{
    if (documents.empty())
        documents.push(new CustomTextDocument);

    return documents.pop();
}

void DocumentCache::push(QTextDocument *doc)
{
    if (doc)
    {
        doc->clear();
        documents.push(doc);
    }
}

DocumentCache &DocumentCache::getInstance()
{
    static DocumentCache instance;
    return instance;
}
