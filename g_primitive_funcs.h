#pragma once
#include <iostream>
#include <vector>
#include <functional>

namespace Gore {

	


	
	template<class T>
	class Vector {
	public:
		using iterator = T*;
		iterator begin() { return stor; }
		iterator end() { return stor + offset; }
	private:
		T* stor;
		size_t offset;
		size_t byte_size;
		size_t allocd;
	public:
		Vector() {
			stor = (T*)std::malloc(sizeof(T));
			offset = 0;
			byte_size = 0;
			allocd = sizeof(T);
		}
		void push_back(T in) {
			if (byte_size + sizeof(T) > allocd) {
				//we realloc double the amount of data to reduce realloc calls
				allocd = allocd << 1;
				T* temp = (T*)std::realloc(stor, allocd);
				if (temp != NULL) {
					stor = temp;
				}
			}
			*(stor + offset) = in;
			offset++;
			byte_size += sizeof(T);
		}
		void erase(int n) {
			//move entire chunk back by one and offset is reduced by one 
			std::memcpy(stor + n, stor + n + 1, (offset - n) * sizeof(T));
			offset--;
			byte_size -= sizeof(T);
		}
		size_t size() {
			return offset;
		}
		void reserve(size_t n) {
			T* temp = (T*)std::realloc(stor, allocd + (sizeof(T) * n));
			if (temp != NULL) {
				stor = temp;
			}
			allocd += sizeof(T) * n;
		}
		void pop_back() {
			//don't need to touch data will just be copied over next push_back
			byte_size -= sizeof(T);
			offset--;
		}
		void insert(int n, T in) {
			//checking if enough space to memcpy over, branchless? 
			if (byte_size + sizeof(T) > allocd) {
				T* temp = (T*)std::realloc(stor, allocd + sizeof(T));
				if (temp != NULL) {
					stor = temp;
				}
				allocd += sizeof(T);
			}
			//copy current data over by 1 offset
			std::memcpy(stor + n + 1, stor + n, (offset - n) * sizeof(T));
			*(stor + n) = in;
			byte_size += sizeof(T);
			offset++;
		}
		void clear() {
			//just reset entire block of data, without deallocating it
			std::memset(stor, 0, offset * sizeof(T));
			offset = 0;
			byte_size = 0;
		}
		T& operator[] (int n) {
			return stor[n];
		}
		~Vector() {
			std::free(stor);
		}
	};


	//use offset within data instead to avoid the pointer being made useless if realloc moves chunk
	struct StoreElement {
		size_t offset;
		size_t size;
		size_t type;
	};
	//returns char pointer which you convert to the struct you want
	struct ReturnElement {
		char* data;
		size_t type;
	};
	//keep track of your own types, I don't feel like writing a reflection system
	//could probably optimize this pretty hard
	class MultiVector {
	private:
		//stores the location of data and its size
		std::vector<StoreElement> elements;
		//stores gaps made by erase
		std::vector<StoreElement> gaps;
		//where to write in the data
		char* index;
		//the number of bytes written
		size_t size;
		//the number of bytes allocated
		size_t allocd;
		//the actual pointer to the data
		char* stor;
	public:
		MultiVector() {
			stor = (char*)std::malloc(32);
			index = stor;
			size = 0;
			allocd = 32;
		}
		void push_back(char* data, size_t insize, size_t type) {
			size_t off = size;
			//checking if item fits into any of the gaps
			for (int i = 0; i < (int)gaps.size(); i++) {
				if (gaps[i].size <= insize) {
					char* t = stor + gaps[i].offset;
					off = gaps[i].offset;
					for (size_t j = 0; j < gaps[i].size; j++) {
						*t = data[j];
						t++;
					}
					StoreElement st = { off, insize, type };
					elements.push_back(st);
					gaps.erase(gaps.begin() + i);
					return;
				}
			}
			if (size + insize > allocd) {
				char* temp = (char*)std::realloc(stor, allocd + insize);
				if (temp != NULL) {
					stor = temp;
					index = stor;
					//moving index back to where it was before we reallocd the memory
					for (int i = 0; i < (int)size; i++) {
						index++;
					}
				}
				allocd += insize;
			}
			for (size_t i = 0; i < insize; i++) {
				*index = data[i];
				index++;
				size++;
			}
			StoreElement st = { off, insize, type };
			elements.push_back(st);
		}
		void erase(size_t n) {
			gaps.push_back(elements[n]);
			elements.erase(elements.begin() + n);
		}
		void pop_back() {
			size -= elements[elements.size() - 1].size;
			index -= elements[elements.size() - 1].size;
			elements.erase(elements.begin() + (elements.size() - 1));
		}
		void reserve(size_t in) {
			char* temp = (char*)std::realloc(stor, allocd + in);
			if (temp != NULL) {
				stor = temp;
				index = stor;
				for (int i = 0; i < (int)size; i++) {
					index++;
				}
			}
			allocd += in;
		}
		void insert(int n, char* data, size_t insize, size_t type) {
			size_t off = size;
			//checking if item fits into any of the gaps
			for (int i = 0; i < (int)gaps.size(); i++) {
				if (gaps[i].size <= insize) {
					char* t = stor + gaps[i].offset;
					off = gaps[i].offset;
					for (size_t j = 0; j < gaps[i].size; j++) {
						*t = data[j];
						t++;
					}
					StoreElement st = { off, insize, type };
					elements.insert(elements.begin() + n, st);
					gaps.erase(gaps.begin() + i);
					return;
				}
			}
			if (size + insize > allocd) {
				char* temp = (char*)std::realloc(stor, allocd + insize);
				if (temp != NULL) {
					stor = temp;
					index = stor;
					//moving index back to where it was before we reallocd the memory
					for (int i = 0; i < (int)size; i++) {
						index++;
					}
				}
				allocd += insize;
			}
			for (size_t i = 0; i < insize; i++) {
				*index = data[i];
				index++;
				size++;
			}
			elements.insert(elements.begin() + n, {off, insize, type});
			size += insize;
			index += insize;
		}
		void clear() {
			elements.clear();
			size = 0;
			index = stor;
		}

		size_t getSize() {
			return elements.size();
		}

		ReturnElement operator[](int n) {
			return { stor + elements[n].offset, elements[n].type };
		}

		const ReturnElement operator[](int n) const {
			return{ stor + elements[n].offset, elements[n].type };
		}

	};
	
	template<class F>
	struct FObj {
		F* current;
		FObj<F>* next;
		std::string name;
	};
	template<class W> 
	class ForwardList {
	private:
		FObj<W>* object = nullptr;
	public:
		//will have problem with remove function if pointer isn't created with new
		void insert(W* item, std::string name) {
			FObj<W>* nt = new FObj<W>;
			nt->name = name;
			nt->current = item;
			nt->next = object;
			object = nt;
		}
		void insert(W item, std::string name) {
			FObj<W>* nt = new FObj<W>;
			W* it = new W;
			*it = item;
			nt->name = name;
			nt->current = it;
			nt->next = object;
			object = nt;
		}
		W* search(std::string name) {
			FObj<W>* ptr = object;
			while (ptr != nullptr) {
				if (ptr->name.compare(name.c_str()) == 0) {
					return ptr->current;
				}
				ptr = ptr->next;
			}
			return nullptr;
		}
		void removeFObj(std::string name) {
			FObj<W>* ptr = object;
			FObj<W>* prev = ptr;
			int c = 0;
			while (ptr != nullptr) {
				if (ptr->name.compare(name.c_str()) == 0) {
					prev->next = ptr->next;
					ptr->name.clear();
					if (c == 0) {
						object = ptr->next;
					}
					delete ptr;
					return;
				}
				prev = ptr;
				ptr = ptr->next;
				c++;
			}
		}
		void removeBoth(std::string name) {
			FObj<W>* ptr = object;
			FObj<W>* prev = ptr;
			int c = 0;
			while (ptr != nullptr) {
				if (ptr->name.compare(name.c_str()) == 0) {
					prev->next = ptr->next;
					ptr->name.clear();
					if (c == 0) {
						object = ptr->next;
					}
					delete ptr->current;
					delete ptr;
					return;
				}
				prev = ptr;
				ptr = ptr->next;
				c++;
			}
		}
		FObj<W>* getHead() {
			return object;
		}
	};
	template<typename F>
	struct FBObj {
		F* current;
		FBObj<F>* prev;
		FBObj<F>* next;
		std::string name;
	};
	//forwards and backwards linked list
	template<class T>
	class FBList {
	public:
		FBObj<T>* obj;
		FBList() {
			obj = nullptr;
		}
		void insert(T in_obj, std::string name) {
			FBObj<T>* nt = new FBObj<T>;
			nt->current = new T;
			*nt->current = in_obj;
			nt->name = name;
			nt->next = obj;
			nt->prev = nullptr;
			(obj != nullptr) ? obj->prev = nt : nt;
			obj = nt;
		}
		void insert(T* in_obj, std::string name) {
			FBObj<T>* nt = new FBObj<T>;
			nt->current = in_obj;
			nt->name = name;
			nt->next = obj;
			nt->prev = nullptr;
			(obj != nullptr) ? obj->prev = nt : nt;
			obj = nt;
		}
		T* search(std::string name) {
			FBObj<T>* ptr = obj;
			while (ptr != nullptr) {
				if (name.compare(ptr->name) == 0) {
					return ptr->current;
				}
				ptr = ptr->next;
			}
			return nullptr;
		}
		void removeBoth(std::string name) {
			FBObj<T>* ptr = obj;
			while (ptr != nullptr) {
				if (name.compare(ptr->name) == 0) {
					FBObj<T>* t1 = ptr->prev;
					FBObj<T>* t2 = ptr->next;
					(t1 != nullptr) ? t1->next = t2 : obj = t2;
					(t2 != nullptr) ? t2->prev = t1 : t2;
					ptr->name.clear();
					delete ptr->current;
					delete ptr;
					return;
				}
				ptr = ptr->next;
			}
		}
		void removeLinkObj(std::string name) {
			FBObj<T>* ptr = obj;
			while (ptr != nullptr) {
				if (name.compare(ptr->name) == 0) {
					FBObj<T>* t1 = ptr->prev;
					FBObj<T>* t2 = ptr->next;
					(t1 != nullptr) ? t1->next = t2 : obj = t2;
					(t2 != nullptr) ? t2->prev = t1 : t2;
					ptr->name.clear();
					delete ptr;
					return;
				}
				ptr = ptr->next;
			}
		}
		FBObj<T>* getHead() {
			return obj;
		}
	};


	template<typename T>
	struct MapItem {
		std::string key;
		T item;
		MapItem<T>* next;
	};
	//gonna be more inefficent than a specially designed version
	template<typename T>
	class HashMap {
	private:
		std::vector<MapItem<T>*> buckets;
		std::function<int(std::string)> hash_func;
	public:
		HashMap() {

		}
		~HashMap() {

			buckets.clear();
		}
		void insert(std::string f, T item) {
			int n = hash_func(f);
			if (n > int(buckets.size()) - 1) {
				int dif = n - (buckets.size() - 1);
				for (dif; dif > 0; dif--) {
					buckets.push_back(nullptr);
				}
			}
			MapItem<T>* temp = buckets[n];
			buckets[n] = new MapItem<T>;
			buckets[n]->key = f;
			buckets[n]->item = item;
			buckets[n]->next = temp;
			
		}
		T* get(std::string f) {
			int n = hash_func(f);
			if (n > int(buckets.size()) - 1 || buckets[n] == nullptr) {
				return nullptr;
			}
			if (buckets[n]->key.compare(f) != 0) {
				MapItem<T>* ptr = buckets[n];
				while (ptr != nullptr) {
					if (ptr->key.compare(f) == 0) {
						return &ptr->item;
					}
					ptr = ptr->next;
				}
				return nullptr;
			}
			return &buckets[n]->item;
		}
		void setHashFunction(std::function<int(std::string)> in) {
			buckets.clear();
			hash_func = in;
		}
		void reserve(size_t n) {
			buckets.reserve(n);
		}

	};
}