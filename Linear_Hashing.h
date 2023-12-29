#ifndef LINEAR_HASHING_H
#define LINEAR_HASHING_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 3>
class Linear_Hashing {
public:
    class Iterator;
    using value_type = Key;
    using key_type = Key;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using const_iterator = Iterator;
    using iterator = const_iterator;
    using key_equal = std::equal_to<key_type>;                       
    using hasher = std::hash<key_type>;                              

private:
    struct Bucket;
    struct Element {
        key_type key;
        bool used{ false };
        bool end{ false };

    };
    struct Bucket {
        Element* data;
        size_type num_elements;
        Bucket* overflow; 
        Bucket** row; 
        bool last {false};


        bool full() { return num_elements == N; }

        Element* end(){return data+N;}

        Bucket() : num_elements(0), overflow(nullptr), row(nullptr) {
            data = new Element[N+1];
        }

        Bucket(Bucket** row) : num_elements(0), overflow(nullptr), row(row) {
            data = new Element[N+1];
        }

        void deleteOverflowChain(Bucket* bucket) {
            if (bucket->overflow){
                deleteOverflowChain(bucket->overflow);
                bucket->overflow = nullptr;
            }
            delete bucket;
        }

        ~Bucket() {
            delete[] data;
            if (overflow) deleteOverflowChain(overflow);

        }

    };
    Bucket** table{ nullptr };
    size_type d{ 0 };
    size_type nextToSplit{ 0 };

    size_type row_size{ 0 };

    size_type table_elements{0};

    std::pair<Element*,Bucket*> add(const key_type&);
    bool checkOverflow(const key_type&);
    void split();
    std::pair<Element*,Bucket*> locate(const key_type& key) const;
    size_type h(const key_type&) const;
    size_type g(const key_type&) const { return 1; }
    size_type rows() const { return (size_type)(1 << d) + nextToSplit; }
    size_type table_size() const { return rows() * N; }
    void reserve(const key_type&);
    void rehash();
    void allocate(size_type n);

public:
    Linear_Hashing() : table{ new Bucket*[1]{} }, d{ 0 }, nextToSplit{ 0 }, row_size{ 1 } {
        table[0] = new Bucket(&table[0]);
        (table[0])->last = true;
    }                                                         
    Linear_Hashing(std::initializer_list<key_type> ilist) : Linear_Hashing{} { insert(ilist); }                    
    template<typename InputIt> Linear_Hashing(InputIt first, InputIt last) : Linear_Hashing{} { insert(first, last); }     
    Linear_Hashing(const Linear_Hashing &other): Linear_Hashing{other.begin(), other.end()} { }

    ~Linear_Hashing() {
    for (size_t i{ 0 }; i < rows(); ++i) {
        delete table[i];
    }
    delete[] table;
    }

    Linear_Hashing &operator=(const Linear_Hashing &other);
    Linear_Hashing &operator=(std::initializer_list<key_type> ilist);

    size_type size() const { return table_elements; }                                              
    bool empty() const { return table_elements == 0; }                                                    

    void insert(std::initializer_list<key_type> ilist) { insert(ilist.begin(), ilist.end()); }                 
    std::pair<iterator,bool> insert(const key_type &key);
    template<typename InputIt> void insert(InputIt first, InputIt last); 

    void clear();
    size_type erase(const key_type &key);

    size_type count(const key_type& key) const { return locate(key).first != nullptr; }                          
    iterator find(const key_type &key) const;

    void swap(Linear_Hashing &other);

    const_iterator begin() const { return const_iterator{ table[0]->data, table[0] }; }
    const_iterator end() const {
        Element* el = nullptr;
        Bucket* bucket = table[rows() - 1];
        while (bucket->overflow)
            bucket = bucket->overflow;
        el = bucket->end();
        return const_iterator{ el, bucket };
    }

    void dump(std::ostream& o = std::cerr) const;

    friend bool operator==(const Linear_Hashing &lhs, const Linear_Hashing &rhs) {
        if (lhs.table_elements != rhs.table_elements) return false;
        for (const auto &k: lhs) if (!rhs.count(k)) return false;
        return true;
    }
    friend bool operator!=(const Linear_Hashing &lhs, const Linear_Hashing &rhs) { return !(lhs == rhs); }
};

template <typename Key, size_t N>
std::pair<typename Linear_Hashing<Key, N>::Element*, typename Linear_Hashing<Key, N>::Bucket*> Linear_Hashing<Key,N>::add(const key_type& key) {
    Element *el{nullptr};
    size_type idx{ h(key) };
    Bucket* bucket = table[idx];
    while (bucket->overflow)
        bucket = bucket->overflow;
    if (!bucket->full()){
        for (size_type i = 0; i < N; i++){
            if(!bucket->data[i].used) {
                bucket->data[i] = { key , true };
                el = &(bucket->data[i]);
                break;
            }
        }
    } 
    else {
        Bucket* newOverflow = new Bucket(&table[idx]);
        bucket->overflow = newOverflow;
        bucket = bucket->overflow;
        for (size_type i = 0; i < N; i++){
            if(!bucket->data[i].used) {
                bucket->data[i] = { key , true };
                el = &(bucket->data[i]);
                break;
            }
        }
    }
    ++bucket->num_elements;
    ++table_elements;
    return {el, bucket};
}

template <typename Key, size_t N>
bool Linear_Hashing<Key, N>::checkOverflow(const key_type& key) {
    size_type idx{ h(key) };
    Bucket* bucket = table[idx];
    do {
        if (bucket->full() && !bucket->overflow) return true;
        bucket = bucket->overflow;
    } while (bucket);
    return false;
}

template <typename Key, size_t N>
void Linear_Hashing<Key, N>::split() {
    if (rows() >= row_size) allocate(2 * rows() + 1);
    (table[rows() - 1])->last = false;
    if (++nextToSplit >= (size_type)(1 << d)) { d++; nextToSplit = 0; }
    table[rows()-1] = new Bucket(&table[rows()-1]);
    
    (table[rows() - 1])->last = true;

}

template <typename Key, size_t N>
void Linear_Hashing<Key, N>::allocate(size_type n) {
    if (n < rows()) {
        throw std::runtime_error("Error.");
    }
    if (n == 0) {
        for (size_t i = 0; i < rows(); ++i) {
            delete table[i]; 
        }
        delete[] table;
        table = nullptr;
        row_size = 0;
        d = 0;
        nextToSplit = 0;
    } else {
        Bucket** buf = new Bucket*[n];
        for (size_type i = 0; i < rows(); ++i) {
            buf[i] = table[i];
            Bucket* bucket = buf[i];
            while (bucket){
                bucket->row = &buf[i];
                bucket = bucket->overflow;
            }
        }
        row_size = n;
        delete[] table;
        table = buf;
    }
}

template <typename Key, size_t N>
size_t Linear_Hashing<Key, N>::h(const key_type& key) const {
    size_type temp_d{ d };
    if ((hasher{}(key) % (size_type)(1 << d)) < nextToSplit) temp_d++;
    return hasher{}(key) % (size_type)(1 << temp_d);
}

template <typename Key, size_t N>
std::pair<typename Linear_Hashing<Key, N>::Element*, typename Linear_Hashing<Key, N>::Bucket*> Linear_Hashing<Key,N>::locate(const key_type& key) const {
    size_type idx{ h(key) };
    Bucket* bucket = table[idx];
    Bucket* tableBucket = bucket;
    do {
        for (size_type i{ 0 }; i < N; i++)
            if (bucket->data[i].used && key_equal{}(bucket->data[i].key, key)) return {&(bucket->data[i]), bucket};
        bucket = bucket->overflow;
    } while (bucket && bucket != tableBucket);
    return {nullptr, bucket};
}

template <typename Key, size_t N>
void Linear_Hashing<Key, N>::reserve(const key_type& n) {
    if (!checkOverflow(n)) return;
    split();
    rehash();
}

template <typename Key, size_t N>
template<typename InputIt> void Linear_Hashing<Key, N>::insert(InputIt first, InputIt last) {
    for (auto it{ first }; it != last; ++it) {
        if (!count(*it)) {
            reserve(*it);
            add(*it);
        }
    }
}


template <typename Key, size_t N>
void Linear_Hashing<Key, N>::rehash() {
    size_type old_d = (nextToSplit == 0) ? d - 1 : d;
    size_type old_nextToSplit = (nextToSplit == 0) ? (1 << old_d) - 1 : nextToSplit - 1;

    Bucket* bucket = table[old_nextToSplit];
    table[old_nextToSplit] = new Bucket(&table[old_nextToSplit]);
    Bucket* tempBucket = bucket;

    do {
        for (size_type i{ 0 }; i < N; i++) {
            if (tempBucket->data[i].used){
                add(tempBucket->data[i].key);
                table_elements--;
            }

        }
        tempBucket=tempBucket->overflow;
    } while (tempBucket);
    delete bucket;
}

template <typename Key, size_t N>
void Linear_Hashing<Key, N>::dump(std::ostream& o) const {
    o << "table_size = " << table_size() << ", current_size = "  << table_elements << ", row_size = " << row_size <<
         ", rows = " << rows()<< ", d = " << d << ", nextToSplit = " << nextToSplit << std::endl;
    for (size_type r{ 0 }; r < rows(); ++r) {
        o << r << " |";
        Bucket* bucket = table[r];
        do {
            for (size_type i{ 0 }; i < N+1; i++) {
                if (bucket->data[i].used) o << bucket->data[i].key;
                else o << "_";
                o << "|";
            }
            bucket = bucket->overflow;
            if (bucket) o << "<-|";
        } while (bucket);
        o << std::endl;
    }
}

template <typename Key, size_t N>
Linear_Hashing<Key,N> &Linear_Hashing<Key,N>::operator=(const Linear_Hashing &other) {
    if (this == &other) return *this;
    Linear_Hashing tmp{other};
    swap(tmp);
    return *this;
}

template <typename Key, size_t N>
Linear_Hashing<Key,N> &Linear_Hashing<Key,N>::operator=(std::initializer_list<key_type> ilist) {
    Linear_Hashing tmp{ilist};
    swap(tmp);
    return *this;
}

template <typename Key, size_t N>
std::pair<typename Linear_Hashing<Key,N>::iterator,bool> Linear_Hashing<Key,N>::insert(const key_type &key) {
    auto p1 {locate(key)};
    if (p1.first) return {iterator{p1.first,p1.second},false};
    reserve(key);
    auto p2 {add(key)};
    return {iterator{p2.first,p2.second},true};
}

template <typename Key, size_t N>
void Linear_Hashing<Key,N>::clear() {
    Linear_Hashing tmp;
    swap(tmp);
}

template <typename Key, size_t N>
typename Linear_Hashing<Key,N>::size_type Linear_Hashing<Key,N>::erase(const key_type &key) {
    auto p {locate(key)};
    if (p.first) {
        p.first->used = false;
        --(p.second->num_elements);
        --table_elements;
        return 1;
    }
    return 0;
}

template <typename Key, size_t N>
typename Linear_Hashing<Key,N>::iterator Linear_Hashing<Key,N>::find(const key_type &key) const {
    auto p {locate(key)};
    if (p.first) return iterator{p.first,p.second};
    return end();
}

template <typename Key, size_t N>
void Linear_Hashing<Key,N>::swap(Linear_Hashing &other) {
  using std::swap;
  swap(table, other.table);
  swap(d, other.d);
  swap(nextToSplit, other.nextToSplit);
  swap(row_size, other.row_size);
  swap(table_elements, other.table_elements);
}

template <typename Key, size_t N>
class Linear_Hashing<Key, N>::Iterator {
    Element *e;
    Bucket *b;
    bool endReached(){
        return e == b->end() && !b->overflow && (*(b->row))->last;
    }
    void skip(){
        while (!e->used && !endReached()){
            if(!jumpToNext()) ++e;
        }
    }

    bool jumpToNext(){
        if(e == b->end()){
            if(!b->overflow){
                if(!(*(b->row))->last){
                    Bucket** nextRow = (b->row)+1;
                    b = *nextRow;
                    e = b->data;
                }  
            }
            else{
                b = b->overflow;
                    e = b->data;
            } 
            return true;
        }
        return false;
    }

public:
    using value_type = Key;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type&;
    using pointer = const value_type*;
    using iterator_category = std::forward_iterator_tag;

  explicit Iterator(Element *e = nullptr, Bucket *b = nullptr ): e{e}, b(b) { if (e)  skip(); }
  reference operator*() const { return e->key; }
  pointer operator->() const { return &e->key; }
  Iterator &operator++() { ++e; skip(); return *this; }
  Iterator operator++(int) { auto rc {*this}; ++*this; return rc; } 
  friend bool operator==(const Iterator &lhs, const Iterator &rhs) { return lhs.e == rhs.e; }
  friend bool operator!=(const Iterator &lhs, const Iterator &rhs) { return !(lhs == rhs); }
  Element* getEl(){return e;}
};

template <typename Key, size_t N>
void swap(Linear_Hashing<Key, N>& lhs, Linear_Hashing<Key, N>& rhs) { lhs.swap(rhs); }

#endif