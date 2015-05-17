#ifndef TOXID_H
#define TOXID_H

#include <QString>

/*!
 * \brief This class represents a Tox ID as specified at:
 *        https://libtoxcore.so/core_concepts.html
 */
class ToxId
{
public:
    ToxId(); ///< The default constructor. Creates an empty Tox ID.
    ToxId(const ToxId& other); ///< The copy constructor.
    ToxId(const QString& id); ///< Create a Tox ID from QString.
                              /// If the given id is not a valid Tox ID, then:
                              /// publicKey == id and noSpam == "" == checkSum.

    bool operator==(const ToxId& other) const; ///< Compares only publicKey.
    bool operator!=(const ToxId& other) const; ///< Compares only publicKey.
    bool isActiveProfile() const; ///< Returns true if this Tox ID is equals to
                                  /// the Tox ID of the currently active profile.
    QString toString() const; ///< Returns the Tox ID as QString.
    void clear(); ///< Clears all elements of the Tox ID.

    static bool isToxId(const QString& id); ///< Returns true if id is a valid Tox ID.

public:
    QString publicKey;
    QString noSpam;
    QString checkSum;
};

#endif // TOXID_H
