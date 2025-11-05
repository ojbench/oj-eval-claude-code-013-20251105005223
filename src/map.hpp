/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
  * You can use sjtu::map as value_type by typedef.
    */
   typedef pair<const Key, T> value_type;

  private:
   // Red-Black Tree node structure
   struct Node {
       value_type data;
       Node *left, *right, *parent;
       bool color; // true for red, false for black

       Node(const value_type &val, Node *p = nullptr)
           : data(val), left(nullptr), right(nullptr), parent(p), color(true) {}
   };

   Node *root;
   size_t tree_size;
   Compare comp;

   // Helper functions for Red-Black Tree operations
   void leftRotate(Node *x) {
       Node *y = x->right;
       x->right = y->left;
       if (y->left != nullptr) {
           y->left->parent = x;
       }
       y->parent = x->parent;
       if (x->parent == nullptr) {
           root = y;
       } else if (x == x->parent->left) {
           x->parent->left = y;
       } else {
           x->parent->right = y;
       }
       y->left = x;
       x->parent = y;
   }

   void rightRotate(Node *x) {
       Node *y = x->left;
       x->left = y->right;
       if (y->right != nullptr) {
           y->right->parent = x;
       }
       y->parent = x->parent;
       if (x->parent == nullptr) {
           root = y;
       } else if (x == x->parent->right) {
           x->parent->right = y;
       } else {
           x->parent->left = y;
       }
       y->right = x;
       x->parent = y;
   }

   void fixInsert(Node *z) {
       while (z != root && z->parent->color) {
           if (z->parent == z->parent->parent->left) {
               Node *y = z->parent->parent->right;
               if (y != nullptr && y->color) {
                   z->parent->color = false;
                   y->color = false;
                   z->parent->parent->color = true;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->right) {
                       z = z->parent;
                       leftRotate(z);
                   }
                   z->parent->color = false;
                   z->parent->parent->color = true;
                   rightRotate(z->parent->parent);
               }
           } else {
               Node *y = z->parent->parent->left;
               if (y != nullptr && y->color) {
                   z->parent->color = false;
                   y->color = false;
                   z->parent->parent->color = true;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->left) {
                       z = z->parent;
                       rightRotate(z);
                   }
                   z->parent->color = false;
                   z->parent->parent->color = true;
                   leftRotate(z->parent->parent);
               }
           }
       }
       root->color = false;
   }

   void transplant(Node *u, Node *v) {
       if (u->parent == nullptr) {
           root = v;
       } else if (u == u->parent->left) {
           u->parent->left = v;
       } else {
           u->parent->right = v;
       }
       if (v != nullptr) {
           v->parent = u->parent;
       }
   }

   void fixDelete(Node *x) {
       while (x != root && (x == nullptr || !x->color)) {
           if (x == x->parent->left) {
               Node *w = x->parent->right;
               if (w->color) {
                   w->color = false;
                   x->parent->color = true;
                   leftRotate(x->parent);
                   w = x->parent->right;
               }
               if ((w->left == nullptr || !w->left->color) &&
                   (w->right == nullptr || !w->right->color)) {
                   w->color = true;
                   x = x->parent;
               } else {
                   if (w->right == nullptr || !w->right->color) {
                       if (w->left != nullptr) w->left->color = false;
                       w->color = true;
                       rightRotate(w);
                       w = x->parent->right;
                   }
                   w->color = x->parent->color;
                   x->parent->color = false;
                   if (w->right != nullptr) w->right->color = false;
                   leftRotate(x->parent);
                   x = root;
               }
           } else {
               Node *w = x->parent->left;
               if (w->color) {
                   w->color = false;
                   x->parent->color = true;
                   rightRotate(x->parent);
                   w = x->parent->left;
               }
               if ((w->right == nullptr || !w->right->color) &&
                   (w->left == nullptr || !w->left->color)) {
                   w->color = true;
                   x = x->parent;
               } else {
                   if (w->left == nullptr || !w->left->color) {
                       if (w->right != nullptr) w->right->color = false;
                       w->color = true;
                       leftRotate(w);
                       w = x->parent->left;
                   }
                   w->color = x->parent->color;
                   x->parent->color = false;
                   if (w->left != nullptr) w->left->color = false;
                   rightRotate(x->parent);
                   x = root;
               }
           }
       }
       if (x != nullptr) x->color = false;
   }

   Node* findNode(const Key &key) const {
       Node *current = root;
       while (current != nullptr) {
           if (comp(key, current->data.first)) {
               current = current->left;
           } else if (comp(current->data.first, key)) {
               current = current->right;
           } else {
               return current;
           }
       }
       return nullptr;
   }

   Node* minimum(Node *node) const {
       while (node != nullptr && node->left != nullptr) {
           node = node->left;
       }
       return node;
   }

   Node* maximum(Node *node) const {
       while (node != nullptr && node->right != nullptr) {
           node = node->right;
       }
       return node;
   }

   void clearTree(Node *node) {
       if (node == nullptr) return;
       clearTree(node->left);
       clearTree(node->right);
       delete node;
   }

   Node* copyTree(Node *node, Node *parent) {
       if (node == nullptr) return nullptr;
       Node *newNode = new Node(node->data, parent);
       newNode->color = node->color;
       newNode->left = copyTree(node->left, newNode);
       newNode->right = copyTree(node->right, newNode);
       return newNode;
   }

  public:
   /**
  * see BidirectionalIterator at CppReference for help.
  *
  * if there is anything wrong throw invalid_iterator.
  *     like it = map.begin(); --it;
  *       or it = map.end(); ++end();
    */
   class const_iterator;
   class iterator {
      private:
       Node *node;
       const map *container;

      public:
       iterator() : node(nullptr), container(nullptr) {}

       iterator(Node *n, const map *c) : node(n), container(c) {}

       iterator(const iterator &other) : node(other.node), container(other.container) {}

       /**
    * TODO iter++
        */
       iterator operator++(int) {
           iterator temp = *this;
           ++(*this);
           return temp;
       }

       /**
    * TODO ++iter
        */
       iterator &operator++() {
           if (node == nullptr) {
               throw invalid_iterator();
           }

           if (node->right != nullptr) {
               node = node->right;
               while (node->left != nullptr) {
                   node = node->left;
               }
           } else {
               Node *parent = node->parent;
               while (parent != nullptr && node == parent->right) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return *this;
       }

       /**
    * TODO iter--
        */
       iterator operator--(int) {
           iterator temp = *this;
           --(*this);
           return temp;
       }

       /**
    * TODO --iter
        */
       iterator &operator--() {
           if (node == nullptr) {
               if (container == nullptr || container->root == nullptr) {
                   throw invalid_iterator();
               }
               node = container->maximum(container->root);
           } else {
               if (node->left != nullptr) {
                   node = node->left;
                   while (node->right != nullptr) {
                       node = node->right;
                   }
               } else {
                   Node *parent = node->parent;
                   while (parent != nullptr && node == parent->left) {
                       node = parent;
                       parent = parent->parent;
                   }
                   node = parent;
               }
           }
           if (node == nullptr) {
               throw invalid_iterator();
           }
           return *this;
       }

       /**
    * a operator to check whether two iterators are same (pointing to the same memory).
        */
       value_type &operator*() const {
           if (node == nullptr) {
               throw invalid_iterator();
           }
           return node->data;
       }

       bool operator==(const iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       bool operator==(const const_iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       /**
    * some other operator for iterator.
        */
       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       /**
    * for the support of it->first.
    * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
        */
       value_type *operator->() const {
           if (node == nullptr) {
               throw invalid_iterator();
           }
           return &(node->data);
       }

       friend class map;
       friend class const_iterator;
   };

   class const_iterator {
       // it should has similar member method as iterator.
       //  and it should be able to construct from an iterator.
      private:
       const Node *node;
       const map *container;

      public:
       const_iterator() : node(nullptr), container(nullptr) {}

       const_iterator(const Node *n, const map *c) : node(n), container(c) {}

       const_iterator(const const_iterator &other) : node(other.node), container(other.container) {}

       const_iterator(const iterator &other) : node(other.node), container(other.container) {}

       const_iterator operator++(int) {
           const_iterator temp = *this;
           ++(*this);
           return temp;
       }

       const_iterator &operator++() {
           if (node == nullptr) {
               throw invalid_iterator();
           }

           if (node->right != nullptr) {
               node = node->right;
               while (node->left != nullptr) {
                   node = node->left;
               }
           } else {
               const Node *parent = node->parent;
               while (parent != nullptr && node == parent->right) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return *this;
       }

       const_iterator operator--(int) {
           const_iterator temp = *this;
           --(*this);
           return temp;
       }

       const_iterator &operator--() {
           if (node == nullptr) {
               if (container == nullptr || container->root == nullptr) {
                   throw invalid_iterator();
               }
               node = container->maximum(container->root);
           } else {
               if (node->left != nullptr) {
                   node = node->left;
                   while (node->right != nullptr) {
                       node = node->right;
                   }
               } else {
                   const Node *parent = node->parent;
                   while (parent != nullptr && node == parent->left) {
                       node = parent;
                       parent = parent->parent;
                   }
                   node = parent;
               }
           }
           if (node == nullptr) {
               throw invalid_iterator();
           }
           return *this;
       }

       const value_type &operator*() const {
           if (node == nullptr) {
               throw invalid_iterator();
           }
           return node->data;
       }

       bool operator==(const iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       bool operator==(const const_iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       const value_type *operator->() const {
           if (node == nullptr) {
               throw invalid_iterator();
           }
           return &(node->data);
       }

       friend class map;
   };

   /**
  * TODO two constructors
    */
   map() : root(nullptr), tree_size(0), comp() {}

   map(const map &other) : root(nullptr), tree_size(0), comp(other.comp) {
       root = copyTree(other.root, nullptr);
       tree_size = other.tree_size;
   }

   /**
  * TODO assignment operator
    */
   map &operator=(const map &other) {
       if (this != &other) {
           clearTree(root);
           root = copyTree(other.root, nullptr);
           tree_size = other.tree_size;
           comp = other.comp;
       }
       return *this;
   }

   /**
  * TODO Destructors
    */
   ~map() {
       clearTree(root);
   }

   /**
  * TODO
  * access specified element with bounds checking
  * Returns a reference to the mapped value of the element with key equivalent to key.
  * If no such element exists, an exception of type `index_out_of_bound'
    */
   T &at(const Key &key) {
       Node *node = findNode(key);
       if (node == nullptr) {
           throw index_out_of_bound();
       }
       return node->data.second;
   }

   const T &at(const Key &key) const {
       const Node *node = findNode(key);
       if (node == nullptr) {
           throw index_out_of_bound();
       }
       return node->data.second;
   }

   /**
  * TODO
  * access specified element
  * Returns a reference to the value that is mapped to a key equivalent to key,
  *   performing an insertion if such key does not already exist.
    */
   T &operator[](const Key &key) {
       Node *node = findNode(key);
       if (node != nullptr) {
           return node->data.second;
       }

       // Insert new element with default constructed T
       T default_value = T();
       pair<iterator, bool> result = insert(value_type(key, default_value));
       return result.first->second;
   }

   /**
  * behave like at() throw index_out_of_bound if such key does not exist.
    */
   const T &operator[](const Key &key) const {
       return at(key);
   }

   /**
  * return a iterator to the beginning
    */
   iterator begin() {
       return iterator(minimum(root), this);
   }

   const_iterator cbegin() const {
       return const_iterator(minimum(root), this);
   }

   /**
  * return a iterator to the end
  * in fact, it returns past-the-end.
    */
   iterator end() {
       return iterator(nullptr, this);
   }

   const_iterator cend() const {
       return const_iterator(nullptr, this);
   }

   /**
  * checks whether the container is empty
  * return true if empty, otherwise false.
    */
   bool empty() const {
       return tree_size == 0;
   }

   /**
  * returns the number of elements.
    */
   size_t size() const {
       return tree_size;
   }

   /**
  * clears the contents
    */
   void clear() {
       clearTree(root);
       root = nullptr;
       tree_size = 0;
   }

   /**
  * insert an element.
  * return a pair, the first of the pair is
  *   the iterator to the new element (or the element that prevented the insertion),
  *   the second one is true if insert successfully, or false.
    */
   pair<iterator, bool> insert(const value_type &value) {
       Node *parent = nullptr;
       Node *current = root;

       while (current != nullptr) {
           parent = current;
           if (comp(value.first, current->data.first)) {
               current = current->left;
           } else if (comp(current->data.first, value.first)) {
               current = current->right;
           } else {
               // Key already exists
               return pair<iterator, bool>(iterator(current, this), false);
           }
       }

       Node *newNode = new Node(value, parent);
       if (parent == nullptr) {
           root = newNode;
       } else if (comp(value.first, parent->data.first)) {
           parent->left = newNode;
       } else {
           parent->right = newNode;
       }

       fixInsert(newNode);
       tree_size++;
       return pair<iterator, bool>(iterator(newNode, this), true);
   }

   /**
  * erase the element at pos.
  *
  * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
    */
   void erase(iterator pos) {
       if (pos.container != this || pos.node == nullptr) {
           throw invalid_iterator();
       }

       Node *z = pos.node;
       Node *y = z;
       Node *x = nullptr;
       bool y_original_color = y->color;

       if (z->left == nullptr) {
           x = z->right;
           transplant(z, z->right);
       } else if (z->right == nullptr) {
           x = z->left;
           transplant(z, z->left);
       } else {
           y = minimum(z->right);
           y_original_color = y->color;
           x = y->right;
           if (y->parent == z) {
               if (x != nullptr) x->parent = y;
           } else {
               transplant(y, y->right);
               y->right = z->right;
               y->right->parent = y;
           }
           transplant(z, y);
           y->left = z->left;
           y->left->parent = y;
           y->color = z->color;
       }

       if (!y_original_color) {
           fixDelete(x);
       }

       delete z;
       tree_size--;
   }

   /**
  * Returns the number of elements with key
  *   that compares equivalent to the specified argument,
  *   which is either 1 or 0
  *     since this container does not allow duplicates.
  * The default method of check the equivalence is !(a < b || b > a)
    */
   size_t count(const Key &key) const {
       return findNode(key) != nullptr ? 1 : 0;
   }

   /**
  * Finds an element with key equivalent to key.
  * key value of the element to search for.
  * Iterator to an element with key equivalent to key.
  *   If no such element is found, past-the-end (see end()) iterator is returned.
    */
   iterator find(const Key &key) {
       Node *node = findNode(key);
       return iterator(node, this);
   }

   const_iterator find(const Key &key) const {
       const Node *node = findNode(key);
       return const_iterator(node, this);
   }
};

}

#endif