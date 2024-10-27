#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono> 
#include <omp.h>
#include "OpenGLRenderer.h" 

int main(int argc, char* argv[]) {
    const std::size_t N = 200;

    float alpha = 0.6f;
    float delta_x = 1.0f / N;
    float delta_t = 0.5 * alpha * delta_x * delta_x;

    std::vector<std::vector<float>> grid(N, std::vector<float>(N, 0.0f));
    std::vector<std::vector<float>> grid_copy(N, std::vector<float>(N, 0.0f));

    for (std::size_t i = 0; i < N; i++) {
        for (std::size_t j = 0; j < N; j++)
        {
            if (i == 0 || i == N - 1 || j == 0 || j == N - 1)
            {
                grid[i][j] = 0.0f;
            }
            else {
                float distance = std::sqrt((i - N / 2.0f) * (i - N / 2.0f) +
                    (j - N / 2.0f) * (j - N / 2.0f));
                if (distance < 25) {
                    grid[i][j] = 1.0f;
                }
                else {
                    grid[i][j] = 0.0f;
                }
            }
        }
    }

    std::ofstream file("output.csv");
    if (!file) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    OpenGLRenderer renderer(800, 800);
    auto start = std::chrono::high_resolution_clock::now();

    for (std::size_t t = 0; t < 10000 && !renderer.shouldClose(); t++) {
        if (t % 10 == 0) {  
            grid[N / 2][N / 2] += 1.0;  
            grid[N / 2][(N+2) / 2] += 1.0;
            grid[(N+2) / 2][N / 2] += 1.0;
            grid[(N+2) / 2][(N+2) / 2] += 1.0;
        }
		// std::cout << "Iteration: " << t << std::endl;
		// std::cout << "Center temperature: " << grid[N / 2][N / 2] << std::endl;

        #pragma omp parallel for collapse(2)
        for (std::size_t i = 1; i < N - 1; i++) {
            for (std::size_t j = 1; j < N - 1; j++) {
                // std::cout << "OpenMP threads: " << omp_get_num_threads() << std::endl;
                grid_copy[i][j] = grid[i][j] + ((alpha * delta_t) * (((grid[i + 1][j] - 2 * grid[i][j] + grid[i - 1][j]) / (delta_x * delta_x)) + ((grid[i][j + 1] - 2 * grid[i][j] + grid[i][j - 1]) / (delta_x * delta_x))));
            }
        }

        grid.swap(grid_copy);

        renderer.pollEvents();
        renderer.renderGrid(grid);
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;

    file.close();
    std::cout << "Data saved to output.csv" << std::endl;

    return 0;
}