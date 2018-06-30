#ifndef LRU_HASH_MAP_H_
#define LRU_HASH_MAP_H_

#include <list>
#include <iterator>
#include <stdint.h>
#include <std/unordered_map>

static const int kSize = 2;

namespace perf_opt {
namespace container {

template <class Key, class Value, class Hash = std::hash<Key> >
class LRUHashmap {
 public:
  typedef std::list<std::pair<Key, Value*> > List;
  typedef typename List::iterator list_iterator;
  typedef std::unordered_map<Key, list_iterator, Hash> Hashmap;
  typedef typename Hashmap::const_iterator const_iterator;
  typedef typename Hashmap::const_local_iterator const_local_iterator;
  typedef typename Hashmap::size_type size_type;

 public:
  LRUHashmap() : ridx_(0) {}

  ~LRUHashmap() {
    Clear();
  }

  void Clear() {
    ridx_ = 0;
    value_list_.clear();
    map_[ridx_].clear();
    map_[1-ridx_].clear();
  }

  bool Insert(const Key& key,
      Value* new_value, Value** old_value) {
    typename Hashmap::iterator m_it = map_[ridx_].find(key);

    if (m_it != map_[ridx_].end()) {
      return false;
    }

    value_list_.push_front(std::make_pair(key, new_value));

    if (map_[ridx_].size() + 1 >=
        map_[ridx_].bucket_count() * map_[ridx_].max_load_factor()) {
      map_[1-ridx_] = map_[ridx_];

      typename List::iterator l_it = value_list_.begin();
      map_[1-ridx_][key] = l_it;

      ridx_ = 1 - ridx_;
    } else { 
      typename List::iterator l_it = value_list_.begin();
      map_[ridx_][key] = l_it;
    }

    return true;
  }

  bool Get(Key key, const Value** value) const {
    const_iterator m_it = map_[ridx_].find(key);

    if (m_it != map_[ridx_].end()) {
      if (value_list_.end() != m_it->second) {
        *value = m_it->second->second;
      }
      return true;
    }

    return false;
  }

  const Value* Get(Key key) const {
    const_iterator m_it = map_[ridx_].find(key);

    if(m_it == map_[ridx_].end()) {
      return NULL;
    }

    if (value_list_.end() == m_it->second) {
      return NULL;
    }
    
    return m_it->second->second;
  }

  bool Splice(const Key& key) {
    typename Hashmap::iterator m_it = map_[ridx_].find(key);
 
    if (map_[ridx_].end() != m_it) {
      value_list_.splice(value_list_.begin(), value_list_, m_it->second);
      typename List::iterator l_it = value_list_.begin();
      map_[ridx_][key] = l_it;
    }
  }

  bool Delete(const Key& key, Value** old_value) {
    typename Hashmap::iterator m_it = map_[ridx_].find(key);

    if ( map_[ridx_].end() != m_it) {
      if (value_list_.end() != m_it->second) {
        *old_value = m_it->second->second;
      }

      value_list_.erase(m_it->second);
      map_[ridx_].erase(m_it);

      return true;
    }

    return false;
  }

  inline bool Empty() const {
    return map_[ridx_].empty();
  }

  inline size_type Size() const {
    return map_[ridx_].size();
  }

  inline size_type BucketCount() const {
    return map_[ridx_].bucket_count();
  }

  const_iterator begin() const {
    return map_[ridx_].begin();
  }

  const_local_iterator begin(int index) const {
    return map_[ridx_].begin(index);
  }

  const_iterator end() const {
    return map_[ridx_].end();
  }

  const_local_iterator end(int index) const {
    return map_[ridx_].end(index);
  }

  /* must use at which list size > 0 */
  bool list_end_key(Key* key) const {
    if (value_list_.size() > 0) {
      typename List::const_iterator l_it = value_list_.end();
      --l_it;
      *key = l_it->first;
      return true;
    }
    return false;
  }

 private:
  int ridx_;
  List value_list_;
  Hashmap map_[kSize];
};

}  // namespace container
}  // namespace perf_opt

#endif  // LRU_HASHMAP__H_
