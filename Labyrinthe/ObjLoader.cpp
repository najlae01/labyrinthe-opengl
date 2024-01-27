#include "ObjLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <iterator>
#include <vector>

std::vector<float> ObjLoader::buffer;

void ObjLoader::search_data(const std::vector<std::string>& data_values, std::vector<float>& coordinates, const std::string& skip, const std::string& data_type) {
    for (const auto& d : data_values) {
        if (d == skip) {
            continue;
        }
        if (data_type == "float") {
            coordinates.push_back(std::stof(d));
        }
        else if (data_type == "int") {
            coordinates.push_back(std::stoi(d) - 1);
        }
    }
}

void ObjLoader::create_sorted_vertex_buffer(const std::vector<float>& indices_data, const std::vector<float>& vertices, const std::vector<float>& textures, const std::vector<float>& normals) {
    for (size_t i = 0; i < indices_data.size(); i += 3) {
        size_t vertexIndex = static_cast<size_t>(indices_data[i]) * 3;
        size_t textureIndex = static_cast<size_t>(indices_data[i + 1]) * 2;
        size_t normalIndex = static_cast<size_t>(indices_data[i + 2]) * 3;

        buffer.insert(buffer.end(), vertices.begin() + vertexIndex, vertices.begin() + vertexIndex + 3);
        buffer.insert(buffer.end(), textures.begin() + textureIndex, textures.begin() + textureIndex + 2);
        buffer.insert(buffer.end(), normals.begin() + normalIndex, normals.begin() + normalIndex + 3);
    }
}

void ObjLoader::create_unsorted_vertex_buffer(const std::vector<float>& indices_data, const std::vector<float>& vertices, const std::vector<float>& textures, const std::vector<float>& normals) {
    size_t num_verts = vertices.size() / 3;

    for (size_t i1 = 0; i1 < num_verts; ++i1) {
        size_t start = i1 * 3;
        size_t end = start + 3;
        buffer.insert(buffer.end(), vertices.begin() + start, vertices.begin() + end);

        for (size_t i2 = 0; i2 < indices_data.size(); i2 += 3) {
            if (i2 % 3 == 0 && indices_data[i2] == i1) {
                size_t start_tex = indices_data[i2 + 1] * 2;
                size_t end_tex = start_tex + 2;
                buffer.insert(buffer.end(), textures.begin() + start_tex, textures.begin() + end_tex);

                size_t start_norm = indices_data[i2 + 2] * 3;
                size_t end_norm = start_norm + 3;
                buffer.insert(buffer.end(), normals.begin() + start_norm, normals.begin() + end_norm);

                break;
            }
        }
    }
}


void ObjLoader::show_buffer_data() {
    for (size_t i = 0; i < buffer.size(); i += 8) {
        size_t start = i;
        size_t end = start + 8;
        for (size_t j = start; j < end; ++j) {
            std::cout << buffer[j] << " ";
        }
        std::cout << std::endl;
    }
}

std::pair<std::vector<uint32_t>, std::vector<float>> ObjLoader::loadModel(const std::string& file, bool sorted = true) {
    std::vector<float> vert_coords; // will contain all the vertex coordinates
    std::vector<float> tex_coords; // will contain all the texture coordinates
    std::vector<float> norm_coords; // will contain all the vertex normals

    std::vector<float> all_indices; // will contain all the vertex, texture, and normal indices
    std::vector<uint32_t> indices; // will contain the indices for indexed drawing

    std::ifstream f(file);
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        std::vector<std::string> values{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };

        if (values[0] == "v") {
            search_data(values, vert_coords, "v", "float");
        }
        else if (values[0] == "vt") {
            search_data(values, tex_coords, "vt", "float");
        }
        else if (values[0] == "vn") {
            search_data(values, norm_coords, "vn", "float");
        }
        else if (values[0] == "f") {
            for (size_t i = 1; i < values.size(); ++i) {
                std::vector<std::string> val = split(values[i], '/');

                if (!val.empty()) {
                    try {
                        search_data(val, all_indices, "f", "int");
                        size_t index = std::stoul(val[0]) - 1;
                        indices.push_back(static_cast<uint32_t>(index));
                    }
                    catch (const std::invalid_argument& e) {
                        // Handle the error (print a message or throw an exception)
                        std::cerr << "Invalid argument in face index: " << val[0] << std::endl;
                        
                    }
                    catch (const std::out_of_range& e) {
                        // Handle the error (print a message or throw an exception)
                        std::cerr << "Out of range error in face index: " << val[0] << std::endl;

                    }
                }
            }
        }
    }

    if (sorted) {
        // use with glDrawArrays
        create_sorted_vertex_buffer(all_indices, vert_coords, tex_coords, norm_coords);
    }
    else {
        // use with glDrawElements
        create_unsorted_vertex_buffer(all_indices, vert_coords, tex_coords, norm_coords);
    }

    //show_buffer_data();

    std::vector<float> local_buffer = buffer; // create a local copy of the buffer list, otherwise it will overwrite the static field buffer
    buffer.clear(); 

    return { indices, local_buffer };
}

std::vector<std::string> ObjLoader::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

