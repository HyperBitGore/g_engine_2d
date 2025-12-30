#pragma once
#include "common.hpp"
#include <stdexcept>
#define KB32 32768
/*
In other words, if one were to print out the compressed data as
a sequence of bytes, starting with the first byte at the
*right* margin and proceeding to the *left*, with the most-
significant bit of each byte on the left as usual, one would be
able to parse the result from right to left, with fixed-width
elements in the correct MSB-to-LSB order and Huffman codes in
bit-reversed order (i.e., with the first bit of the code in the
relative LSB position).
*/
//this is best explanation here
//https://stackoverflow.com/questions/32419086/the-structure-of-deflate-compressed-block
//https://github.com/ebiggers/libdeflate/blob/master/lib/decompress_template.h
//https://github.com/Artexety/inflatecpp/blob/main/source/huffman_decoder.cc#L237
//https://github.com/madler/infgen
//https://nevesnunes.github.io/blog/2021/09/29/Decompression-Meddlings.html
//https://blog.za3k.com/understanding-gzip-2/

// I might need to be decoding the tree bits, right to left???
    //header, extra bits LSB-MSB
    //huffman code MSB-LSB


class inflate : deflate_compressor {
    private:

    class Bitwrapper {
        private:
        size_t size = 0;
        size_t offset = 0;
        uint8_t bit_offset = 0;
        uint8_t* data;
        std::vector<uint8_t> data_clone;
        public:
        Bitwrapper (void* data, size_t size) {
            this->size = size;
            this->data = (uint8_t*)data;
            for (size_t i = 0; i < size; i++) {
                data_clone.push_back(this->data[i]);
            }
        }
        //copy constructor
        Bitwrapper (const Bitwrapper& wrap) {
            size = wrap.size;
            offset = wrap.size;
            bit_offset = wrap.bit_offset;
            data = wrap.data;
            data_clone = wrap.data_clone;
        }

        Bitwrapper& operator=(const Bitwrapper& wrap) {
            return *this = Bitwrapper(wrap);
        }


        //move constructor
        Bitwrapper (const Bitwrapper&& wrap) noexcept {
            size = wrap.size;
            offset = wrap.size;
            bit_offset = wrap.bit_offset;
            data = wrap.data;
            data_clone = wrap.data_clone;
        }

        Bitwrapper& operator=(const Bitwrapper&& wrap) {
            return *this = Bitwrapper(wrap);
        }

        uint32_t readBits (uint8_t bits) {
            uint32_t val = 0;
            uint32_t total_bits = 0;
            if (offset > size) {
                throw std::runtime_error("Reading bits beyond the alloted buffer size!");
            }
            for (int32_t i = bits; i > 0;) {
                uint32_t remaining = 8 - bit_offset;
                uint32_t to_read  = ((i) < (int32_t)remaining) ? i : remaining;
                uint32_t mask = ((1u << to_read) - 1);
                uint32_t chunk = (data[offset] >> bit_offset) & mask; // mask created here and grab data same line
                val |= (chunk << total_bits); // move captured data to correct spot in val
                bit_offset += to_read ;
                total_bits += to_read ;
                i -= to_read ;
                if (bit_offset > 7) {
                    offset++;
                    bit_offset = 0;
                }
                if (offset > size) {
                    throw std::runtime_error("Reading bits beyond the alloted buffer size!");
                }
            }
            return val;
        }

        uint8_t readByte () {
            bit_offset = 0;
            if (offset > size) {
                throw std::runtime_error("Reading bits beyond the alloted buffer size!");
            }
            return data[offset++];
        }

        void moveByte (bool off = false) {
            if (off) {
                if (bit_offset != 0) {
                    offset++;
                    bit_offset = 0;
                }
                return;
            }
            offset++;
            bit_offset = 0;
        }

        size_t getOffset() {
            if (offset > 0 && bit_offset == 0) {
                return offset - 1;
            }
            return offset;
        }

        size_t getSize() {
            return size;
        }
    };

    static FlatHuffmanTree readCodeLengthTree (Bitwrapper& data, uint32_t hclen) {
         std::vector<Code> codes = {
                {0, 0, 2, 16},
                {0, 0, 3, 17},
                {0, 0, 7, 18},
                {0, 0, 0, 0},
                {0, 0, 0, 8},
                {0, 0, 0, 7},
                {0, 0, 0, 9},
                {0, 0, 0, 6},
                {0, 0, 0, 10},
                {0, 0, 0, 5},
                {0, 0, 0, 11},
                {0, 0, 0, 4},
                {0, 0, 0, 12},
                {0, 0, 0, 3},
                {0, 0, 0, 13},
                {0, 0, 0, 2},
                {0, 0, 0, 14},
                {0, 0, 0, 1},
                {0, 0, 0, 15},
        };
        for (uint32_t i = 0; i < hclen + 4; i++) {
                uint32_t val = data.readBits(3);
                codes[i].len = val;
        }
       
        return FlatHuffmanTree(codes);
    }

    static std::vector<Code> readDynamicTreeCodes (Bitwrapper& data, FlatHuffmanTree code_len, size_t iterations) {
        std::vector<Code> codes;
        uint32_t code = 0;
        int32_t cur_bit = 0;
        Code last_code = {0, 0, 0, 0};
        for (uint32_t i = 0; i < iterations;) {
            code = (code << 1) | data.readBits(1);
            cur_bit++;
            Code cc = code_len.getCodeEncoded(code, cur_bit);
            if (cc.code == code && cc.len == cur_bit && cc.len > 0) {
                uint32_t repeat;
                switch (cc.value) {
                    case 16:
                        repeat = data.readBits(2);
                        for (size_t j = 0; j < (repeat + 3); j++, i++) {
                            codes.push_back({0, last_code.value, 0, (uint16_t)i});
                        }
                    break;
                    case 17:
                        repeat = data.readBits(3);
                        for (size_t j = 0; j < (repeat + 3); j++, i++) {
                            codes.push_back({0, 0, 0, (uint16_t)i});
                        }
                    break;
                    case 18:
                        repeat = data.readBits(7);
                        for (size_t j = 0; j < (repeat + 11); j++, i++) {
                            codes.push_back({0, 0, 0, (uint16_t)i});
                        }
                    break;
                    default:
                        codes.push_back({0, cc.value, 0, (uint16_t)i});
                        last_code = cc;
                        i++;
                }
                code = 0;
                cur_bit = 0;
            }
        }
        return codes;
    }

    static std::pair<FlatHuffmanTree, FlatHuffmanTree> decodeTree (Bitwrapper& data) {
        uint32_t hlit = (data.readBits(5));
        uint32_t hdist = data.readBits(5);
        uint32_t hclen = data.readBits(4);
        FlatHuffmanTree code_len = readCodeLengthTree(data, hclen);
        std::vector<Code> cod = code_len.decode();

        // literal/length
        std::vector<Code> litlength = readDynamicTreeCodes(data, code_len, 257 + hlit);
        FlatHuffmanTree littree = FlatHuffmanTree(litlength);
        std::vector<Code> lit_codes = littree.decode();
        // distance
        std::vector<Code> distcodes = readDynamicTreeCodes(data, code_len, 1 + hdist);

        FlatHuffmanTree disttree = FlatHuffmanTree(distcodes);
        return std::pair<FlatHuffmanTree, FlatHuffmanTree>(littree, disttree);
    }
    // https://stackoverflow.com/questions/62827971/can-deflate-only-compress-duplicate-strings-up-to-32-kib-apart
    static void decompressHuffmanBlock (Bitwrapper& data, std::vector<uint8_t>& buffer, FlatHuffmanTree tree, FlatHuffmanTree dist_tree) {
        uint32_t code = 0;
        uint8_t cur_bit = 0;
        RangeLookup rl = generateLengthLookup();
        RangeLookup dl = generateDistanceLookup();
        while (true) {
            code = (code << 1) | data.readBits(1);
            cur_bit++;
            Code c = tree.getCodeEncoded(code, cur_bit);
            if (c.value < 256 && cur_bit == c.len) {
                buffer.push_back((uint8_t)c.value);
                code = 0;
                cur_bit = 0;
            }
            else if (c.value == 256) {
                break;
            } else if (c.value > 256 && cur_bit == c.len) {
                Range r = rl.findCode(c.value);
                uint32_t length = r.start;
                if (r.extra_bits > 0) {
                    uint32_t extra = data.readBits(r.extra_bits);
                    length += extra;
                }
                // decode dist code
                uint32_t dist_code = 0;
                Code dss;
                for (size_t bits = 0; bits < 16; bits++) {
                    dist_code = (dist_code << 1) | data.readBits(1);
                    Code ds = dist_tree.getCodeEncoded(dist_code, bits+1);
                    if (ds.len == (bits+1)) {
                        dss = ds;
                        break;
                    }
                }
                // read full distance code
                Range d = dl.findCode(dss.value);
                uint32_t distance = d.start;
                if (d.extra_bits > 0) {
                    uint32_t extra = data.readBits(d.extra_bits);
                    distance += extra;
                }
                // copy the uncompressed data here using the distance length pair
                for (size_t i = buffer.size() - distance, j = 0; j < length && i < buffer.size(); j++, i++) {
                    buffer.push_back(buffer[i]);
                }
                cur_bit = 0;
            }

        }
    }
    public:

    static size_t decompressZlib (void* in, size_t in_size, void* out, size_t out_size) {
        // idc about zlib data :)
        uint32_t change = 2;
        uint8_t v = extract1BitLeft(*(uint8_t*)in + 1, 5);
        // fdict set, skip the other four bytes with the checksum
        if (v) {
            change = 6;
        }
        return decompress((char*)in + change, in_size - change, out, out_size);
    }

    // done
    static size_t decompress (void* in, size_t in_size, void* out, size_t out_size) {
        Bitwrapper dat = Bitwrapper(in, in_size);
        uint8_t* out_data = (uint8_t*)out;
        //creating default huffman tree
        std::vector <Code> fixed_codes = generateFixedCodes();
        std::vector <Code> fixed_dist_codes = generateFixedDistanceCodes();
        FlatHuffmanTree fixed_huffman(fixed_codes);
        FlatHuffmanTree fixed_dist_huffman(fixed_dist_codes);
        
        uint32_t it = 0;
        uint32_t ot = 0;
        std::vector<uint8_t> buffer;
        while (true) {
            uint8_t final = dat.readBits(1);
            uint8_t type = dat.readBits(2);
            FlatHuffmanTree used_tree = fixed_huffman;
            FlatHuffmanTree used_dist = fixed_dist_huffman;
            switch (type) {
                case 0:
                {
                    dat.moveByte(true);
                    uint16_t len = dat.readBits(16);
                    it += 2;
                    uint16_t nlen = dat.readBits(16);
                    it += 2;
                    for (size_t i = 0; i < len; i++) {
                        buffer.push_back(dat.readByte());
                    }
                }
                break;
                case 2:
                    {
                        std::pair<FlatHuffmanTree, FlatHuffmanTree> trees = decodeTree(dat);
                        used_tree = trees.first;
                        used_dist = trees.second;
                    }
                case 1:
                    decompressHuffmanBlock(dat, buffer, used_tree, used_dist);
                break;
            }
            if (final) {
                break;
            }
            else if (buffer.size() >= out_size || it >= in_size) {
                return 0;
            }
        }
        for (auto& i : buffer) {
            out_data[ot++] = i;
        }
        return ot;
    }

    static std::vector<uint8_t> decompressZlib (void* in, size_t in_size) {
        // idc about zlib data :)
        uint32_t change = 2;
        uint8_t v = extract1BitLeft(*(uint8_t*)in + 1, 5);
        // fdict set, skip the other four bytes with the checksum
        if (v) {
            change = 6;
        }
        return decompress(((char*)in) + change, in_size - change);
    }

    static std::vector<uint8_t> decompress (void* in, size_t in_size) {
        Bitwrapper dat = Bitwrapper(in, in_size);
        //creating default huffman tree
        std::vector <Code> fixed_codes = generateFixedCodes();
        std::vector <Code> fixed_dist_codes = generateFixedDistanceCodes();
        FlatHuffmanTree fixed_huffman(fixed_codes);
        FlatHuffmanTree fixed_dist_huffman(fixed_dist_codes);
        
        std::vector<uint8_t> buffer;
        while (true) {
            uint8_t final = dat.readBits(1);
            uint8_t type = dat.readBits(2);
            FlatHuffmanTree used_tree = fixed_huffman;
            FlatHuffmanTree used_dist = fixed_dist_huffman;
            switch (type) {
                case 0:
                {
                    dat.moveByte(true);
                    uint16_t len = dat.readBits(16);
                    uint16_t nlen = dat.readBits(16);
                    for (size_t i = 0; i < len; i++) {
                        buffer.push_back(dat.readByte());
                    }
                }
                break;
                case 2:
                    {
                        std::pair<FlatHuffmanTree, FlatHuffmanTree> trees = decodeTree(dat);
                        used_tree = trees.first;
                        used_dist = trees.second;
                    }
                case 1:
                    decompressHuffmanBlock(dat, buffer, used_tree, used_dist);
                break;
            }
            if (final) {
                break;
            } else if (dat.getOffset() > in_size) {
                return {};
            }
        }
        if (buffer.size() < in_size) {
            return {};
        }
        return buffer;
    }

    // done
    static size_t decompress (std::string file_path, std::string new_file) {
        //creating default huffman tree
        std::vector <Code> fixed_codes = generateFixedCodes();
        std::vector <Code> fixed_dist_codes = generateFixedDistanceCodes();
        FlatHuffmanTree fixed_huffman(fixed_codes);
        FlatHuffmanTree fixed_dist_huffman(fixed_dist_codes);
        
        uint8_t read_buffer[KB32];
        std::vector<uint8_t> out_buffer;
        std::ifstream f;
        f.open(file_path, std::ios::binary);
        std::ofstream out_file;
        out_file.open(new_file.c_str(), std::ios::binary);
        size_t size = 0;
        while (true) {
            std::memset(read_buffer, 0, KB32);
            out_buffer.clear();
            // read 32kb from the file then 
            f.read((char*)read_buffer, KB32);
            std::streamsize read = f.gcount();
            Bitwrapper dat(read_buffer, (size_t)read);
            uint8_t final = dat.readBits(1);
            uint8_t type = dat.readBits(2);
            FlatHuffmanTree used_tree = fixed_huffman;
            FlatHuffmanTree used_dist = fixed_dist_huffman;
            switch (type) {
                case 0:
                {
                    dat.moveByte(true);
                    uint16_t len = dat.readBits(16);
                    uint16_t nlen = dat.readBits(16);
                    for (size_t i = 0; i < len; i++) {
                        out_buffer.push_back(dat.readByte());
                    }
                }
                break;
                case 2:
                    {
                        std::pair<FlatHuffmanTree, FlatHuffmanTree> trees = decodeTree(dat);
                        used_tree = trees.first;
                        used_dist = trees.second;
                    }
                case 1:
                    decompressHuffmanBlock(dat, out_buffer, used_tree, used_dist);
                break;
            }
            out_file.write((char*)out_buffer.data(), out_buffer.size());
            size += out_buffer.size();
            if (final) {
                break;
            }
        }
        out_file.close();
        f.close();
        return size;
    }
};