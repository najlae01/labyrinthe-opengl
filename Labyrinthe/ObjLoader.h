#ifndef OBJLOADER_H_INCLUDED
#define OBJLOADER_H_INCLUDED

#include <vector>
#include <iostream>


class ObjLoader {
public:

    static void search_data(const std::vector<std::string>&, std::vector<float>&, const std::string&, const std::string&);

    static void create_sorted_vertex_buffer(const std::vector<float>&, const std::vector<float>&, const std::vector<float>&, const std::vector<float>&);

    static void create_unsorted_vertex_buffer(const std::vector<float>&, const std::vector<float>&, const std::vector<float>&, const std::vector<float>&);

    static void show_buffer_data();

    static std::pair<std::vector<uint32_t>, std::vector<float>> loadModel(const std::string&, bool);

private:
    static std::vector<float> buffer;
    static std::vector<std::string> split(const std::string&, char);
};



#endif // OBJLOADER_H_INCLUDED
