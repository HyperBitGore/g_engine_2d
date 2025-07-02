#include <cstddef>
#include <vector>
#include <functional>

template<typename T, typename G>
	struct MapItem {
		G key;
		T item;
		MapItem<T, G>* next;
	};
	//gonna be more inefficent than a specially designed version
	template<typename T, typename G>
	class HashMap {
	private:
		std::vector<MapItem<T, G>*> buckets;
		std::function<int(G)> hash_func;
	public:
		HashMap() {

		}
		~HashMap() {
			//actually need to clean up news here tf
			for (size_t i = 0; i < buckets.size(); i++) {
				if (buckets[i] != nullptr) {
					MapItem<T, G>* cur = buckets[i];
					while (cur != nullptr) {
						MapItem<T, G>* next = cur->next;
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
				for (dif; dif > 0; dif--) {
					buckets.push_back(nullptr);
				}
			}
			MapItem<T, G>* temp = buckets[n];
			buckets[n] = new MapItem<T, G>;
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
				MapItem<T, G>* ptr = buckets[n];
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
				MapItem<T, G>* ptr = buckets[n]->next;
				MapItem<T, G>* last = buckets[n];
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
			MapItem<T, G>* next = buckets[n]->next;
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
				MapItem<T, G>* ptr = buckets[n]->next;
				MapItem<T, G>* last = buckets[n];
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
			MapItem<T, G>* next = buckets[n]->next;
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
		std::vector<MapItem<T, G>*>& getBuckets(){
			return buckets;
		}

	};
