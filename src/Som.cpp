#include "Som.h"

Som::Som(std::string filename, int Iteration, double startLearniningRate)
    :dataNum(0), dataDim(0), total_iteration(Iteration), m_startLearningRate(startLearniningRate), m_latticeSize(32),
    current_iteration(0), isFinished(false), m_NeighbourhoodRadius(0.0), m_influence(0.0), time2update(100)
{
    m_MapRadius = m_latticeSize / 2.0;
    m_TimeConstant = total_iteration / log(m_MapRadius);
    m_LearningRate = m_startLearningRate;
    srand(time(NULL));
    readData(filename);
    init_latticeGL();
}

Som::~Som() {
}

void Som::update(int Iteration, double startLearniningRate, int latticeSize, int time2update) {
    total_iteration = Iteration;
    m_startLearningRate = startLearniningRate;
    m_latticeSize = latticeSize;
    this->time2update = time2update;
    m_MapRadius = latticeSize / 2.0;
    m_TimeConstant = total_iteration / log(m_MapRadius);
    m_LearningRate = m_startLearningRate;
    current_iteration = 0;
    isFinished = false;
    gen_2D_cylinder_lattice();
}

void Som::readData(std::string filename) {
    std::string f = "assets\\" + filename;
    std::ifstream file(f);
    if(!file.is_open()) {
        std::cerr << "Error: file not found" << std::endl;
        return;
    }
    
    file >> dataNum >> dataDim;
    assert(dataNum > 0 && dataDim > 0);

    minPoint = glm::dvec3(1e9, 1e9, 1e9);
    maxPoint = glm::dvec3(-1e9, -1e9, -1e9);

    data.resize(dataNum, std::vector<double>(dataDim, 0.0));
    for(int i = 0; i < dataNum; i++) {
        for(int j = 0; j < dataDim; j++) {
            file >> data[i][j];
            minPoint[j] = std::min(minPoint[j], data[i][j]);
            maxPoint[j] = std::max(maxPoint[j], data[i][j]);
        }
    }

    midPos = (minPoint + maxPoint) / 2.0;

    file.close();

    gen_2D_cylinder_lattice();
}

void Som::gen_2D_cylinder_lattice() {
    lattice.clear();
    lattice.resize(m_latticeSize, std::vector<std::vector<double>>(m_latticeSize, std::vector<double>(dataDim, 0.0)));
    glm::dvec2 center = glm::vec2{midPos.x, midPos.z};
    double radius = glm::distance(glm::vec2{minPoint.x, minPoint.z}, glm::vec2{maxPoint.x, maxPoint.z}) / 2.0;
    double angle = 360.0 / (double)m_latticeSize;
    for(int i = 0; i < m_latticeSize; i++) {
        for(int j = 0; j < m_latticeSize; j++) {
            lattice[i][j][0] = center.x + radius * cos(glm::radians(angle * j));
            lattice[i][j][1] = i * (maxPoint.y - minPoint.y) / m_latticeSize + minPoint.y;
            lattice[i][j][2] = center.y + radius * sin(glm::radians(angle * j));
            // lattice[i][j][0] = rand() % 300;
            // lattice[i][j][1] = rand() % 300;
            // lattice[i][j][2] = rand() % 300;
        }
    }

}

void Som::update_lattice_vertex() {
    lattice_vertex.clear();
    for(int i = 0; i < m_latticeSize - 1; ++i) {
        for(int j = 0; j < m_latticeSize; ++j) {
            lattice_vertex.push_back(lattice[i][j][0]);
            lattice_vertex.push_back(lattice[i][j][1]);
            lattice_vertex.push_back(lattice[i][j][2]);
            
            lattice_vertex.push_back(lattice[i][(j + 1) % m_latticeSize][0]);
            lattice_vertex.push_back(lattice[i][(j + 1) % m_latticeSize][1]);
            lattice_vertex.push_back(lattice[i][(j + 1) % m_latticeSize][2]);

            lattice_vertex.push_back(lattice[(i + 1) % m_latticeSize][(j + 1) % m_latticeSize][0]);
            lattice_vertex.push_back(lattice[(i + 1) % m_latticeSize][(j + 1) % m_latticeSize][1]);
            lattice_vertex.push_back(lattice[(i + 1) % m_latticeSize][(j + 1) % m_latticeSize][2]);

            lattice_vertex.push_back(lattice[(i + 1) % m_latticeSize][(j + 1) % m_latticeSize][0]);
            lattice_vertex.push_back(lattice[(i + 1) % m_latticeSize][(j + 1) % m_latticeSize][1]);
            lattice_vertex.push_back(lattice[(i + 1) % m_latticeSize][(j + 1) % m_latticeSize][2]);
            
            lattice_vertex.push_back(lattice[(i + 1) % m_latticeSize][j][0]);
            lattice_vertex.push_back(lattice[(i + 1) % m_latticeSize][j][1]);
            lattice_vertex.push_back(lattice[(i + 1) % m_latticeSize][j][2]);

            lattice_vertex.push_back(lattice[i][j][0]);
            lattice_vertex.push_back(lattice[i][j][1]);
            lattice_vertex.push_back(lattice[i][j][2]);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, lattice_vbo);
    glBufferData(GL_ARRAY_BUFFER, lattice_vertex.size() * sizeof(float), lattice_vertex.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Som::init_latticeGL() {

    glGenVertexArrays(1, &lattice_vao);
    glBindVertexArray(lattice_vao);

    glGenBuffers(1, &lattice_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, lattice_vbo);
    glBufferData(GL_ARRAY_BUFFER, lattice_vertex.size() * sizeof(float), lattice_vertex.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    update_lattice_vertex();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Som::draw_lattice() {
    // glLineWidth(10.0f);
    glBindVertexArray(lattice_vao);
    glDrawArrays(GL_TRIANGLES, 0, lattice_vertex.size() / 3);
    glBindVertexArray(0);
}

void Som::train() {
    if(isFinished) return;
    if(current_iteration++ <= total_iteration) {
        double r = rand() / (double)RAND_MAX;
        int data_idx = (int)(r * (dataNum - 1));
        glm::ivec2 bmu = FindBMU(data_idx);
        m_NeighbourhoodRadius = m_MapRadius * exp(-(double)current_iteration / m_TimeConstant);

        for(int i = 0; i < m_latticeSize; ++i) {
            for(int j = 0; j < m_latticeSize; ++j) {
                int relative_x = i;
                int relative_y = j;
                int half = m_latticeSize / 2;

                if (bmu.y <= half)
                {
                    if (j >= half + bmu.y)
                        relative_y = bmu.y + (m_latticeSize - j);
                    else
                        relative_y = j - bmu.y;
                }
                else if (bmu.y > half)
                {
                    if (j >= bmu.y - half)
                        relative_y = bmu.y - j;
                    else
                        relative_y = j + (m_latticeSize - bmu.y);
                }
                relative_x = i - bmu.x;

                double dist = sqrt(relative_x * relative_x + relative_y * relative_y);
                if(dist < m_NeighbourhoodRadius) {
                    m_influence = exp(-dist / (2 * m_NeighbourhoodRadius));
                    update_weight(lattice[i][j], data[data_idx]);
                }
            }
        }

        m_LearningRate = m_startLearningRate * exp(-(double)current_iteration / total_iteration);

        if(current_iteration % 100 == 0) {
            update_lattice_vertex();
        }
    }

    if(total_iteration == current_iteration) {
        update_lattice_vertex();
        std::cout << "Training is finished\n";
        isFinished = true;
    }
}

glm::ivec2 Som::FindBMU(int data_idx) {
    double min_dist = DBL_MAX;
    glm::ivec2 bmu = glm::ivec2(0, 0);
    for(int i = 0; i < m_latticeSize; ++i) {
        for(int j = 0; j < m_latticeSize; ++j) {
            double dist = clac_distance(lattice[i][j], data[data_idx]);
            if(dist < min_dist) {
                min_dist = dist;
                bmu = glm::ivec2(i, j);
            }
        }
    }
    return bmu;
}

double Som::clac_distance(std::vector<double> &lattice_data, std::vector<double> &data) {
    double dist = 0.0;
    for(int i = 0; i < dataDim; ++i) {
        dist += (lattice_data[i] - data[i]) * (lattice_data[i] - data[i]);
    }
    return dist;
}

void Som::update_weight(std::vector<double> &lattice_data, std::vector<double> &data) {
    for(int i = 0; i < dataDim; ++i) {
        lattice_data[i] = lattice_data[i] + m_LearningRate * m_influence * (data[i] - lattice_data[i]);
    }
}