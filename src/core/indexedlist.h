#ifndef INDEXEDLIST_H
#define INDEXEDLIST_H

#include <vector>
#include <algorithm>

/// More or less a hashmap, indexed by the noexcept move-only value type's operator int
/// Really nice to store our ToxCall objects.
template <typename T>
class IndexedList
{
public:
    explicit IndexedList() = default;

    // Qt
    inline bool isEmpty() { return v.empty(); }
    bool contains(int i) { return std::find_if(begin(), end(), [i](T& t){return (int)t == i;}) != end(); }
    void remove(int i) { v.erase(std::remove_if(begin(), end(), [i](T& t){return (int)t == i;}), end()); }
    T& operator[](int i)
    {
        iterator it = std::find_if(begin(), end(), [i](T& t){return (int)t == i;});
        if (it == end())
            it = insert({});
        return *it;
    }

    // STL
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    inline iterator begin() { return v.begin(); }
    inline const_iterator begin() const { return v.begin(); }
    inline const_iterator cbegin() const { return v.cbegin(); }
    inline iterator end() { return v.end(); }
    inline const_iterator end() const { return v.end(); }
    inline const_iterator cend() const { return v.cend(); }
    inline iterator erase(iterator pos) { return v.erase(pos); }
    inline iterator erase(iterator first, iterator last) { return v.erase(first, last); }
    inline iterator insert(T&& value) { v.push_back(std::move(value)); return --v.end(); }

private:
    std::vector<T> v;
};

#endif // INDEXEDLIST_H
