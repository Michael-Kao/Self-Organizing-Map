#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <unordered_set>
#include <ctime>
#include <cstdlib>
#include <utility>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Som {
public:
    Som(std::string filename, int Iteration, double startLearniningRate);
    ~Som();
    void draw();
    void draw_lattice();
    void readData(std::string filename);
    glm::dvec3 midPos;
    int current_iteration;
    void train();
    void update(int Iteration, double startLearniningRate, int latticeSize, int time2update);
    bool isFinished;
private:
    std::vector<std::vector<double>> data;
    glm::ivec2 FindBMU(int data_idx);
    double clac_distance(std::vector<double> &lattice_data, std::vector<double> &data);
    void update_weight(std::vector<double> &lattice_data, std::vector<double> &data);
    void gen_2D_cylinder_lattice();
    void init_latticeGL();
    void update_lattice_vertex();
    std::vector<std::vector<std::vector<double>>> lattice;
    std::vector<float> lattice_vertex;
    GLuint lattice_vao, lattice_vbo;
    glm::dvec3 minPoint;
    glm::dvec3 maxPoint;
    int total_iteration;
    double m_startLearningRate;
    double m_influence;
    double m_MapRadius;
    double m_TimeConstant;
    double m_NeighbourhoodRadius;
    double m_LearningRate;
    int m_latticeSize;
    int time2update;
    int dataNum;
    int dataDim;
};