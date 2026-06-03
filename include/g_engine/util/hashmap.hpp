#include <cstddef>
#include <vector>
#include <functional>
namespace gore {
template<typename T, typename G>
	struct mapitem {
		G key;
		T item;
		mapitem<T, G>* next;
	};
	//gonna be more inefficent than a specially designed version
	template<typename T, typename G>
	class hashmap {
	private:
		std::vector<mapitem<T, G>*> buckets;
		std::function<int(G)> hash_func;
	public:
		hashmap() : hash_func(nullptr) {

		}
		~hashmap() {
			clear();
		}
		// copy
		hashmap (const hashmap& m) : hash_func(m.hash_func) {
			for (size_t i = 0; i < m.buckets.size(); i++) {
				mapitem<T, G>* src = m.buckets[i];
				mapitem<T, G>** dst = &buckets.emplace_back(nullptr);
				while (src != nullptr) {
					*dst = new mapitem<T, G>;
					(*dst)->key = src->key;
					(*dst)->item = src->item;
					dst = &(*dst)->next;
					src = src->next;
				}
			}
		}
		// move
		hashmap (hashmap&& m) noexcept : buckets(std::move(m.buckets)), hash_func(std::move(m.hash_func)) {
		}
		// copy assignment
		hashmap& operator=(const hashmap& m) {
			if (this != &m) {
				clear();
				hash_func = m.hash_func;
				for (size_t i = 0; i < m.buckets.size(); i++) {
					mapitem<T, G>* src = m.buckets[i];
					mapitem<T, G>** dst = &buckets.emplace_back(nullptr);
					while (src != nullptr) {
						*dst = new mapitem<T, G>;
						(*dst)->key = src->key;
						(*dst)->item = src->item;
						dst = &(*dst)->next;
						src = src->next;
					}
				}
			}
			return *this;
		}
		// move assignment
		hashmap& operator=(hashmap&& m) noexcept {
			if (this != &m) {
				clear();
				buckets = std::move(m.buckets);
				hash_func = std::move(m.hash_func);
			}
			return *this;
		}

		void clear() {
			for (size_t i = 0; i < buckets.size(); i++) {
				if (buckets[i] != nullptr) {
					mapitem<T, G>* cur = buckets[i];
					while (cur != nullptr) {
						mapitem<T, G>* next = cur->next;
						delete cur;
						cur = next;
					}
				}
			}
			buckets.clear();
		}
		void insert(G f, T item) {
			int n = hash_func(f);
			if (n > int(buckets.size()) - 1) {
				int dif = n - (buckets.size() - 1);
				for (; dif > 0; dif--) {
					buckets.push_back(nullptr);
				}
			}
			mapitem<T, G>* temp = buckets[n];
			buckets[n] = new mapitem<T, G>;
			buckets[n]->key = f;
			buckets[n]->item = item;
			buckets[n]->next = temp;

		}
		T* get(G f) {
			int n = hash_func(f);
			if (n > int(buckets.size()) - 1 || buckets[n] == nullptr) {
				return nullptr;
			}
			if (buckets[n]->key != f) {
				mapitem<T, G>* ptr = buckets[n];
				while (ptr != nullptr) {
					if (ptr->key == f) {
						return &ptr->item;
					}
					ptr = ptr->next;
				}
				return nullptr;
			}
			return &buckets[n]->item;
		}
		bool remove(G f) {
			int n = hash_func(f);
			if (n > int(buckets.size()) - 1 || buckets[n] == nullptr) {
				return false;
			}
			if (buckets[n]->key != f) {
				mapitem<T, G>* ptr = buckets[n]->next;
				mapitem<T, G>* last = buckets[n];
				while (ptr != nullptr) {
					if (ptr->key == f) {
						last->next = ptr->next;
						delete ptr;
						return true;
					}
					last = ptr;
					ptr = ptr->next;
				}
			}
			mapitem<T, G>* next = buckets[n]->next;
			delete buckets[n];
			buckets[n] = next;
			return false;
		}
		bool remove(G f, T* t) {
			int n = hash_func(f);
			if (n > int(buckets.size()) - 1 || buckets[n] == nullptr) {
				return false;
			}
			if (&buckets[n]->item != t) {
				mapitem<T, G>* ptr = buckets[n]->next;
				mapitem<T, G>* last = buckets[n];
				while (ptr != nullptr) {
					if (&ptr->item == t) {
						last->next = ptr->next;
						delete ptr;
						return true;
					}
					last = ptr;
					ptr = ptr->next;
				}
			}
			mapitem<T, G>* next = buckets[n]->next;
			delete buckets[n];
			buckets[n] = next;
			return false;
		}

		void setHashFunction(std::function<int(G)> in) {
			buckets.clear();
			hash_func = in;
		}
		void reserve(size_t n) {
			buckets.reserve(n);
		}
		std::vector<mapitem<T, G>*>& getBuckets(){
			return buckets;
		}

	};
}