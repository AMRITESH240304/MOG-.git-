#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <zlib.h>
#include <vector>
#include "sha1.hpp"

std::vector<char> decompresses_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::vector<char> compressed_data(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    
    // Prepare output buffer
    std::vector<char> decompressed_data(compressed_data.size() * 4);
    uLongf decompressed_size = decompressed_data.size();
    
    // Decompress
    int result = uncompress(
        (Bytef*)decompressed_data.data(),
        &decompressed_size,
        (Bytef*)compressed_data.data(),
        compressed_data.size()
    );

    if (result != Z_OK) {
        std::cerr << "Decompression failed\n";
        exit(EXIT_FAILURE);
    }
    
    decompressed_data.resize(decompressed_size);
    return decompressed_data;
}

int main(int argc, char *argv[])
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here!\n";

     if (argc < 2) {
        std::cerr << "No command provided.\n";
        return EXIT_FAILURE;
    }

    std::string command = argv[1];
    if (command == "init") {
        try {
            std::filesystem::create_directory(".git");
            std::filesystem::create_directory(".git/objects");
            std::filesystem::create_directory(".git/refs");
            std::ofstream headFile(".git/HEAD");
            if (headFile.is_open()) {
                headFile << "ref: refs/heads/main\n";
                headFile.close();
            } else {
                std::cerr << "Failed to create .git/HEAD file.\n";
                return EXIT_FAILURE;
            }
            std::cout << "Initialized git directory\n";
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << e.what() << '\n';
            return EXIT_FAILURE;
        }
    }
    else if(command == "cat-file"){
        if (argc < 4 || std::string(argv[2]) != "-p") {
            std::cerr << "Invalid parameters for cat-file command.\n";
            return EXIT_FAILURE;
        }

        std::string blob_hash = argv[3];
        // std::cout<<blob_hash<<std::endl;
        std::string object_path = ".git/objects/" + blob_hash.substr(0,2) + "/" + blob_hash.substr(2);

        std::vector<char> decompressed = decompresses_file(object_path);
        size_t content_start = 0;
        for (size_t i = 0; i < decompressed.size(); i++) {
            if (decompressed[i] == '\0') {
                content_start = i + 1;
                break;
            }
        }
        // std::cout<<content_start<<std::endl;
        std::cout.write(decompressed.data() + content_start, decompressed.size() - content_start);

    }
    else if(command == "hash-object"){
        // std::cout<<"Not implemented yet\n";
        if(argc < 4 || std::string(argv[2]) != "-w"){
            std::cerr<<"Invalid parameters for hash-object command.\n";
            return EXIT_FAILURE;
        }

        std::string filename = argv[3];

        std::ifstream file(filename, std::ios::binary);
        if(!file.is_open()){
            std::cerr<<"Failed to open file "<<filename<<std::endl;
            return EXIT_FAILURE;
        }

        std::vector<char> content(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        std::string header = "blob " + std::to_string(content.size()) + '\0';

        const std::string input = header + std::string(content.begin(), content.end());

        SHA1 checksum;
        checksum.update(input);
        const std::string hash = checksum.final();
        // std::cout<<hash<<std::endl;

        uLongf compressed_size = compressBound(input.size());
        // std::cout<<compressed_size<<std::endl;
        std::vector<char> compressed_data(compressed_size);
        
        int result = compress(
            (Bytef*)compressed_data.data(),
            &compressed_size,
            (Bytef*)input.data(),
            input.size()
        );
        
        if (result != Z_OK) {
            std::cerr << "Compression failed with error code: " << result << std::endl;
            return EXIT_FAILURE;
        }
        
        std::string object_path = ".git/objects/" + hash.substr(0,2) + "/" + hash.substr(2);
        std::filesystem::create_directories(".git/objects/" + hash.substr(0,2));
        std::ofstream object_file(object_path, std::ios::binary);
        if(!object_file.is_open()){
            std::cerr<<"Failed to create object file "<<object_path<<std::endl;
            return EXIT_FAILURE;
        }

        object_file.write(compressed_data.data(), compressed_size);
        std::cout.write(hash.data(), hash.size());
        object_file.close();
    }
    else if(command == "ls-tree"){
        if (argc < 4 || std::string(argv[2]) != "--name-only") {
            std::cerr << "Invalid parameters for ls-tree command.\n";
            return EXIT_FAILURE;
        }

        std::string tree_hash = argv[3];
        std::string object_path = ".git/objects/" + tree_hash.substr(0,2) + "/" + tree_hash.substr(2);
        // if(!file.is_open()){
        //     std::cerr<<"Failed to open file "<<object_path<<std::endl;
        //     return EXIT_FAILURE;
        // }

        std::vector<char> decompressed = decompresses_file(object_path); 
        
        size_t content_start = 0;
        for (size_t i = 0; i < decompressed.size(); i++) {
            if (decompressed[i] == '\0') {
                content_start = i + 1;
                break;
            }
        }

        size_t i = content_start;
        while (i < decompressed.size()) {
            size_t j = i;
            while (decompressed[j] != '\0') {
                j++;
            }
            std::cout.write(decompressed.data() + i, j - i);
            std::cout << '\n';
            i = j + 21;
        }
        
    }
    else {
        std::cerr << "Unknown command " << command << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
