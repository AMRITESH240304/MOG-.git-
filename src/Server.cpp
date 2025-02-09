#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <zlib.h>
#include <vector>

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

    // Uncomment this block to pass the first stage
    //
    // if (argc < 2) {
    //     std::cerr << "No command provided.\n";
    //     return EXIT_FAILURE;
    // }
    //
    // std::string command = argv[1];
    //
    // if (command == "init") {
    //     try {
    //         std::filesystem::create_directory(".git");
    //         std::filesystem::create_directory(".git/objects");
    //         std::filesystem::create_directory(".git/refs");
    //
    //         std::ofstream headFile(".git/HEAD");
    //         if (headFile.is_open()) {
    //             headFile << "ref: refs/heads/main\n";
    //             headFile.close();
    //         } else {
    //             std::cerr << "Failed to create .git/HEAD file.\n";
    //             return EXIT_FAILURE;
    //         }
    //
    //         std::cout << "Initialized git directory\n";
    //     } catch (const std::filesystem::filesystem_error& e) {
    //         std::cerr << e.what() << '\n';
    //         return EXIT_FAILURE;
    //     }
    // } else {
    //     std::cerr << "Unknown command " << command << '\n';
    //     return EXIT_FAILURE;
    // }
    //
    // return EXIT_SUCCESS;

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
    else {
        std::cerr << "Unknown command " << command << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
