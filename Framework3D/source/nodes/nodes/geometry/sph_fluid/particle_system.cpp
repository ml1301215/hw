#include "particle_system.h"
#include <iostream>

namespace USTC_CG::node_sph_fluid {

using namespace Eigen;
using namespace std;
#define M_PI 3.14159265358979323846

ParticleSystem::ParticleSystem(const MatrixXd &X, const Vector3d &box_min, const Vector3d &box_max)
{
    // Initialize the particles
    for (int i = 0; i < X.rows(); i++) {
        add_particle(X.row(i).transpose(), Particle::FLUID);
    }

    num_fluid_particles_ = particles_.size();

    init_neighbor_search(box_min, box_max); 
}

ParticleSystem::ParticleSystem(const MatrixXd &fluid_particle_X, 
    const MatrixXd& boundary_particle_X, 
    const Vector3d &box_min, const Vector3d &box_max)
{
    // Initialize the particles
    for (int i = 0; i < fluid_particle_X.rows(); i++) {
        add_particle(fluid_particle_X.row(i).transpose(), Particle::FLUID);
    }
    for (int i = 0; i < boundary_particle_X.rows(); i++) {
        add_particle(boundary_particle_X.row(i).transpose(), Particle::BOUNDARY);
    }

    num_fluid_particles_ = fluid_particle_X.rows();
    num_boundary_particles_ = boundary_particle_X.rows();

    Vector3d scaled_box_min = boundary_particle_X.colwise().minCoeff(); 
    Vector3d scaled_box_max = boundary_particle_X.colwise().maxCoeff(); 
    init_neighbor_search(box_min, box_max); 
}


void ParticleSystem::searchNeighbors()
{
    // update the neighbors for each particle
    for (auto &p : particles_) {
        p->neighbors_.clear();
        if (p->type_ == Particle::BOUNDARY) {
            // TODO: do we need to search neighbors for boundary particles? NO; 
            continue;
        }

        // Traverse neighbor grid cells
        auto neighbor_cell_indices = get_neighbor_cell_indices(p->X_);
        for (auto &cell_idx : neighbor_cell_indices) {
            for (auto &q : cells_[cell_idx]) {
                if ((p->X_ - q->X_).norm() < 1.001 * support_radius_) {
                    p->neighbors_.push_back(q);
                }
            }
        }
    }
}

// A hash function mapping spatial position to cell index
unsigned ParticleSystem::pos_to_cell_index(const Vector3d &pos) const
{
    auto xyz = pos_to_cell_xyz(pos);
    return cell_xyz_to_cell_index(xyz[0], xyz[1], xyz[2]);
}

unsigned
ParticleSystem::cell_xyz_to_cell_index(const unsigned x, const unsigned y, const unsigned z) const
{
    return x * n_cell_per_axis_[1] * n_cell_per_axis_[2] + y * n_cell_per_axis_[2] + z;
}

Vector3i ParticleSystem::pos_to_cell_xyz(const Vector3d &pos) const
{
    double eps = 1e-8;
    int x = static_cast<int>(floor((pos[0] - box_min_[0]) / cell_size_ + eps));
    int y = static_cast<int>(floor((pos[1] - box_min_[1]) / cell_size_ + eps));

    // double zz = (pos[2] - box_min_[2]);
    // double tmp_zz = (pos[2] - box_min_[2]) / cell_size_;
    // double floor_tmp_zz = floor(tmp_zz);

    int z = static_cast<int>(floor((pos[2] - box_min_[2]) / cell_size_ + eps));

    return Vector3i(x, y, z);
}

// return center_cell index and its neighbors
std::vector<unsigned> ParticleSystem::get_neighbor_cell_indices(const Vector3d &pos) const
{
    std::vector<unsigned> neighbor_cell_indices;

    auto xyz = pos_to_cell_xyz(pos);
    int x = xyz[0];
    int y = xyz[1];
    int z = xyz[2];

    // compute indices
    const unsigned n_cell = n_cell_per_axis_.prod();
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (int k = -1; k <= 1; k++) {
                if (x + i < 0 || x + i >= n_cell_per_axis_[0] || y + j < 0 ||
                    y + j >= n_cell_per_axis_[1] || z + k < 0 || z + k >= n_cell_per_axis_[2]) {
                    continue;
                }
                unsigned idx = cell_xyz_to_cell_index(x + i, y + j, z + k);
                // Check if valid
                if (idx >= 0 && idx < n_cell) {
                    neighbor_cell_indices.push_back(idx);
                }
            }
        }
    }
    return neighbor_cell_indices;
}

void ParticleSystem::assign_particles_to_cells()
{
    // Clear all cells
    for (auto &cell : cells_) {
        cell.clear();
    }

    // Assign particles to cells
    for (auto &p : particles_) {
        unsigned cell_idx = pos_to_cell_index(p->X_);
        if (cell_idx >= 0 && cell_idx < cells_.size())
            cells_[cell_idx].push_back(p);
        else {
            std::cout << "[assign_particles_to_cells] cell_idx out of range: cell= " << cell_idx;
            std::cout << " particle pos= " << p->X_.transpose() << std::endl;
            std::cout << "n cells per axis = " << n_cell_per_axis_.transpose() << std::endl;
            std::cout << "cell size = " << cell_size_ << std::endl;
            //throw std::runtime_error("cell_idx out of range");
            exit(1);
        }
    }
}

void ParticleSystem::init_neighbor_search(const Vector3d area_min, const Vector3d area_max)
{
    // Compute the number of cells in each axis
    box_max_ = area_min;
    box_min_ = area_max;
    n_cell_per_axis_ = ((box_max_ - box_min_) / cell_size_)
                           .array()
                           .ceil()
                           .cast<int>();  // Extend one more for safety
    // TODO: need to check here box_max is bigger then box_min

    cells_.resize(n_cell_per_axis_[0] * n_cell_per_axis_[1] * n_cell_per_axis_[2]);

    assign_particles_to_cells();
    searchNeighbors();

}

// First, add a particle sample function from a box area, which is needed in node system
MatrixXd ParticleSystem::sample_particle_pos_in_a_box(
    const Vector3d min,
    const Vector3d max,
    const Vector3i n_per_axis,
    const bool sample_2d
)
{
    const Vector3d step = (max - min).array() / n_per_axis.array().cast<double>();
    const int n_particles = n_per_axis.prod();

    // TODO: potential access violation here: more than n_particles particles
    MatrixXd X = MatrixXd::Zero(n_particles, 3);
    for (int i = 0; i < n_per_axis[0]; i++) {
        for (int j = 0; j < n_per_axis[1]; j++) {
            for (int k = 0; k < n_per_axis[2]; k++) {

                double x_coord = min[0] + i * step[0]; 
                
                if (sample_2d) // sim 2d 
                    x_coord = 0.0;

                X.row(i * n_per_axis[1] * n_per_axis[2] + j * n_per_axis[2] + k)
                    << x_coord,
                    min[1] + j * step[1], min[2] + k * step[2];
            }
        }
    }
    return X;
}

MatrixXd ParticleSystem::sample_particle_pos_around_a_box(
    const Vector3d min,
    const Vector3d max,
    const Vector3i n_per_axis,
    const bool sample_2d
)
{
	 Vector3d step = (max - min).array() / n_per_axis.array().cast<double>();
	 const double s = 0.2; // scale factor 
	 Vector3d scaled_min = min - s * (max - min); 
	 Vector3d scaled_max = max + s * (max - min); 

    vector<Vector3d> pos;

    if (sample_2d) 
    {
        scaled_min[0] = 0.0;
        scaled_max[0] = 1.0; 
        step[0] = scaled_max[0] - scaled_min[0] + 1.0; // make sure loop runs only once
    }

	 for (double x = scaled_min[0]; x <= scaled_max[0]; x += step[0]) {
        for (double y = scaled_min[1]; y <= scaled_max[1]; y += step[1]) {
            for (double z = scaled_min[2]; z <= scaled_max[2]; z += step[2]) {
                if (x < min[0] || x > max[0] || y < min[1] || y > max[1] || z < min[2] ||
                    z > max[2]) {
                    pos.push_back(Vector3d{ x, y, z });
                }
            }
        }
    }

     MatrixXd X = MatrixXd::Zero(pos.size(), 3);
     for (auto& p : pos) {
		X.row(&p - &pos[0]) = p.transpose();
	}   

     return X; 
}

void ParticleSystem::add_particle(const Vector3d X, Particle::particleType type)
{
	shared_ptr<Particle> p = make_shared<Particle>();
    p->X_ = X; 
	p->vel_ = Vector3d::Zero();
	p->density_ = 0.0;
	p->pressure_ = 0.0;
    p->type_ = type; 
    p->idx_ = particles_.size(); 
	p->mass_ = particle_mass_;
	particles_.push_back(p);
}

}  // namespace USTC_CG::node_sph_fluid
