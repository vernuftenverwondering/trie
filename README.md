# Trie

a key-value storage for keys that are sequences of elements, e.g. a string or a vector.

Storage in the Trie is efficient in case of overlapping keys. 
Keys should provide stl-style forward iterators that give access to the elements in the sequence through dereferencing (operator*()) and begin() and end() functions to obtain such iterators. 
Data should be default constructible.
The trie does not differentiate between stored keys and subsequences of those keys, each subsequence can be associated with a data element.
Insertion may invalidate iterators.


